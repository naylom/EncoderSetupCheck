/*
   Encoder setup check sketch,  Author Mark Naylor, 2020

   Arduino sketch to help check the inputs are being seen correctly before using the encoder quality check sketch

   First identifies which of first 21 digital pins will support interrupts then looks for 15 seconds (default) for signals with pins in INPUT_PULLUP mode 
   
   and then checks any remaining pins (for another 15 seconds) using INPUT if any signals subsequently seen. Stops after 3 or more pins seen genertaing interrupts

   It uses interrupts to process in the incoming signals and hence requires an Arduino with 3 pins that can generate interrupts. All testing has been done on an Arduino Mega2560.

   Ver 1 - Initial release, Dec 2020

 */

// Configuration items
#define BAUD_RATE                 115200
#define VER                       1
#define CHECK_TIME				  15						// time to wait (in seconds) to see signals 
#define	PINS_LOOKING_FOR		  3							// Number of pins expected to signal, quit when reached
#define SKIP_INPUT_PULLUP_CHECK	  false

// ISR Routine declarations
void ISR0 ( void );
void ISR1 ( void );
void ISR2 ( void );
void ISR3 ( void );
void ISR4 ( void );
void ISR5 ( void );
void ISR6 ( void );
void ISR7 ( void );
void ISR8 ( void );
void ISR9 ( void );
void ISR10 ( void );
void ISR11 ( void );
void ISR12 ( void );
void ISR13 ( void );
void ISR14 ( void );
void ISR15 ( void );
void ISR16 ( void );
void ISR17 ( void );
void ISR18 ( void );
void ISR19 ( void );
void ISR20 ( void );
void ISR21 ( void );
// Array of ISR routines to use
void ( *ListISR [22] )(void)  = 				// List of ISRs for first 22 pins
{
	ISR0,
	ISR1,
	ISR2,
	ISR3,
	ISR4,
	ISR5,
	ISR6,
	ISR7,
	ISR8,
	ISR9,
	ISR10,
	ISR11,
	ISR12,
	ISR13,
	ISR14,
	ISR15,
	ISR16,
	ISR17,
	ISR18,
	ISR19,
	ISR20,
	ISR21
};
#define MAX_PINS_TO_CHECK         (int)min ( NUM_DIGITAL_PINS, sizeof ( ListISR ) / sizeof ( ListISR[0] ) )

class CPinData							// Information about a Pin
{
	// Data
private:
	int						m_iCurrentPinMode;
	int						m_iPinNum;
	volatile bool			m_bHasBeenSignalled;
	volatile unsigned long	m_ulInterruptCount;
	volatile unsigned long	m_ulLastSignalTime;
	volatile unsigned long  m_ulIntervalSum;
	volatile int			m_iInputModeWhenSignalled;
	volatile int			m_iNoiseCount;
	void					(*m_pISR)();		// Interrupt routine for this Pin
public:
	
	// Implementation
private:
	void SetInputModeWhenSignalled ()
	{
		m_iInputModeWhenSignalled = m_iCurrentPinMode;
	}

public:
	CPinData()
	{
		PinNum ( -1 );
		m_bHasBeenSignalled = false;
		m_iInputModeWhenSignalled =  -1;
		m_iCurrentPinMode = -1;
		m_pISR = 0;
		m_ulInterruptCount = 0UL;
		m_ulLastSignalTime = 0UL;
		m_iNoiseCount = 0;
		m_ulIntervalSum = 0;
	}

	~CPinData()
	{
		detachInterrupt ( digitalPinToInterrupt ( m_iPinNum ) );
	}

	// Getter and Setters
	int PinNum() const { return m_iPinNum; }
	void PinNum ( int iNum ) {m_iPinNum = iNum; }
	bool HasSignalled() const { return m_bHasBeenSignalled; }
	int InputModeWhenSignalled () const { return m_iInputModeWhenSignalled; }
	void theISR ( void (*fn)() ) { m_pISR = fn;}
	unsigned long InterruptTime () const { return m_ulLastSignalTime; }
	void InterruptTime ( unsigned long ulTime ) { m_ulLastSignalTime = ulTime; }
	void IncNoise () { m_iNoiseCount++; }
	int Noise () const { return m_iNoiseCount; }
	void IncInterval ( unsigned long ulInc ) { m_ulIntervalSum += ulInc; }
	unsigned long Intervals () { return m_ulIntervalSum; }

	String GetSignalledInputMode () const
	{
		String sResult;
		switch ( m_iInputModeWhenSignalled )
		{
			case INPUT:
				sResult = "INPUT";
				break;

			case INPUT_PULLUP:
				sResult = "INPUT_PULLUP";
				break;

			default:
				sResult = "Unknown";
				break;
		}
		return sResult;
	}
	
	void HasSignalled ( bool bSignal )
	{
		m_bHasBeenSignalled = bSignal;
		if ( bSignal == true )
		{
			// remember mode that pin was in at the time
			SetInputModeWhenSignalled();
		}
	}

	void IncInterruptCount()
	{
		m_ulInterruptCount++;
	}

	unsigned long GetInterruptCount() const
	{
		return m_ulInterruptCount;
	}

	void SetMode ( int iMode )
	{
		pinMode ( m_iPinNum, iMode );
		m_iCurrentPinMode = iMode;
	}

	void StartISR()
	{
		attachInterrupt ( digitalPinToInterrupt ( PinNum() ), m_pISR, FALLING );
	}

	void StopISR ()
	{
		detachInterrupt ( digitalPinToInterrupt ( PinNum() ) );
	}
};

class CBoardPins						// Class to hold and manipulate pins to be checked
{
	// Data
public:
private:
	bool			m_bStart;
	int				m_iNumPins;			// Number of digital pins that support interrupts on this board
	volatile int	m_iNumPinsSignalled;
	CPinData *		m_aPinData [ MAX_PINS_TO_CHECK] ;
	
	// Implementations
private:
public:

	CBoardPins()
	{
		m_iNumPins = 0;
		m_iNumPinsSignalled = 0;
		m_bStart = false;
		// create CPinData instances
		// determine which pins can support interrupts   
		for ( int i = 0 ; i < NUM_DIGITAL_PINS ; i++ )
		{
			if ( digitalPinToInterrupt ( i ) != -1 )
			{
				m_aPinData [ m_iNumPins ] =  new CPinData;
				m_aPinData [ m_iNumPins ]->PinNum ( i );
				m_aPinData [ m_iNumPins++ ]->theISR ( ListISR [ i ] );
			}
		}
	}

	~CBoardPins()
	{
		// delete CPinData instances
		for ( int i = 0 ; i < NumPins() ; i++ )
		{
			delete m_aPinData [ i ];
		}
	}
	// called by ISRs
	void UpdateSignalInfo ( int iIndex )
	{
		if ( m_bStart && m_iNumPinsSignalled < PINS_LOOKING_FOR )
		{
			unsigned long ulTimeNow = micros();			// this does not change when called in an ISR
			// Find matching Index
			for ( int i = 0 ; i < NumPins() ; i++ )
			{
				if ( m_aPinData [ i ]->PinNum() == iIndex )
				{
					// Set has signalled to true
					unsigned long ulLastTime = m_aPinData [ i ]->InterruptTime();
					unsigned long ulInterval = ulTimeNow - ulLastTime;
					if ( ulInterval > 50UL )		// micros gives results that are multiple of 4 micros
					{				
						if ( m_aPinData [ i ]->HasSignalled() == false )
						{
							// first time this pin has signalled since start
							m_iNumPinsSignalled++;
							m_aPinData [ i ]->HasSignalled ( true );
						}
						else
						{
							m_aPinData [ i ]->IncInterval ( ulInterval );
						}
						m_aPinData [ i ]->InterruptTime ( ulTimeNow );
						m_aPinData [ i ]->IncInterruptCount();
					}
					else
					{
						m_aPinData [ i ]->IncNoise();
					}
					break;					
				}
			}
		}
	}
	
	// Getter and Setters
	int NumPins() const { return m_iNumPins; }
	int NumPinsSignalled() const { return m_iNumPinsSignalled; }
	
	void Start () { m_bStart = true; }
	void Stop () { m_bStart = false; }

	int GetPinNumber ( int iIndex ) const
	{
		int iResult = -1;
		if ( iIndex < NumPins() )
		{
			iResult = m_aPinData [ iIndex ]->PinNum();
		}
		return iResult;
	}

	void SetPinsMode ( int iMode )
	{
		for ( int i = 0 ; i < NumPins() ; i++ )
		{
			// check if we still need to get a result for this pin
			if ( m_aPinData [ i ]->HasSignalled() == false )
			{
				m_aPinData [ i ]->SetMode ( iMode );    
			}
		}
	}

	void StartISRs ()
	{
		noInterrupts();
		for ( int i = 0 ; i < NumPins() ; i ++ )
		{
			if ( m_aPinData [ i ]->HasSignalled() == false )
			{
				m_aPinData [ i ]->StartISR();
			}			
		}
		interrupts();
	}

	void StopISRs()
	{
		noInterrupts();
		for ( int i = 0 ; i < NumPins() ; i ++ )
		{
			m_aPinData [ i ]->StopISR();
		}
		interrupts();
		Stop();
	}

	void DisplayResults()
	{
		for ( int i = 0 ; i < NumPins() ; i++ )
  		{
			Serial.print (  "\n Pin " );
			Serial.print ( m_aPinData [ i ]->PinNum() );
			Serial.print ( " attached to interrupt " );
			Serial.print ( digitalPinToInterrupt ( m_aPinData [ i ]->PinNum() ) );
			if ( m_aPinData [ i ]->HasSignalled() == true )
			{
				Serial.print ( " signalled in mode " );
				Serial.print ( m_aPinData [ i ]->GetSignalledInputMode () );
				Serial.print ( " #Signals " );
				Serial.print ( m_aPinData [ i ]->GetInterruptCount () );
				Serial.print ( " #DiscardedSignals " );
				Serial.print ( m_aPinData [ i ]->Noise() );				
			}
			else
			{
				Serial.print ( " did not signal" );
			}
		    Serial.println();
 	 	}
		Serial.println ();
	}
};

CBoardPins myPins; 

void setup ()
{
	Serial.begin (BAUD_RATE);
	delay ( 1000 );			// wait for comms to settle
	// Print config state for this run
	Serial.print  ( "\n\n\n\n\nEncoder Setup Check Ver. " );
	Serial.println ( VER );
	Serial.print ( "Serial output running at " );
	Serial.println ( BAUD_RATE );

	if ( NUM_DIGITAL_PINS > MAX_PINS_TO_CHECK  )
	{
		Serial.print ( "Compatibility warning: This program will only check the first " );
		Serial.print ( MAX_PINS_TO_CHECK );
		Serial.print ( " digital pins, your machine has " );
		Serial.println ( NUM_DIGITAL_PINS );
	}
	Serial.println();

	Serial.print ( myPins.NumPins() );
	Serial.println ( " pin(s) can be used for interrupts as follows:\nPin #\tInt Vector" );
	for ( int i = 0 ; i < myPins.NumPins() ; i++ )
	{
		int iPin = myPins.GetPinNumber ( i );
		if ( iPin != -1 )
		{
			Serial.print ( iPin );
			Serial.print ( "\t" );
			Serial.println ( digitalPinToInterrupt ( iPin ) );
		}
	}

	Serial.println ( "\nStarting to look for signals on pins" );

	Serial.println();	
}

void loop()
{
	static int iCountSec = 0;
	switch  ( iCountSec )
	{
		case 0:
			// check if this is to be skipped
			if ( SKIP_INPUT_PULLUP_CHECK ==  true )
			{
				iCountSec = CHECK_TIME - 1;
			}
			else
			{			
				// put in PULLUP mode
				Serial.println ( "\nChecking INPUT_PULLUP mode" );

				myPins.StopISRs();
				myPins.SetPinsMode ( INPUT_PULLUP );
				myPins.StartISRs ();

				Serial.println ( "Please turn encoder" );
				myPins.Start();
			}
			break;
		
		case CHECK_TIME * 2:				// checking every .5 seconds
			// put in input mode
			Serial.println ( "\nChecking INPUT mode, spurious readings can occur if no external pullups on pins" );
			
			myPins.StopISRs ();
			myPins.SetPinsMode ( INPUT );
			myPins.StartISRs ();

			Serial.println ( "Please continue to turn encoder" );
			myPins.Start();
			break;
		
		case CHECK_TIME * 4:			// // checking every .5 seconds
			// end of program
			myPins.StopISRs();
			Serial.println ();
			Serial.println ( "\nFinished testing" );
			myPins.DisplayResults ();
			TerminateProgram ( "Program ending" );
			break;

		default:
			// check if 3 pins have signalled
			Serial.print ( "." );
			if ( myPins.NumPinsSignalled () >= PINS_LOOKING_FOR )
			{
				myPins.StopISRs();
				myPins.DisplayResults ();
				TerminateProgram ( "Minimum number of pins found, terminating program");
			}
			break;
	}
	delay ( 500 );		// check every half second
	iCountSec++;
}

void TerminateProgram ( String pErrMsg )
{
  Serial.println ( pErrMsg );
  Serial.flush ();
  exit (0);
}

/*
 * ISR code implementation
 */
// Each ISR must be unique as there is no way to pass in a parameter
void ISR0 ( void )
{
	myPins.UpdateSignalInfo ( 0 );
}
void ISR1 ( void )
{
	myPins.UpdateSignalInfo ( 1 );
}
void ISR2 ( void )
{
	myPins.UpdateSignalInfo ( 2 );
}
void ISR3 ( void )
{
	myPins.UpdateSignalInfo ( 3 );
}
void ISR4 ( void )
{
	myPins.UpdateSignalInfo ( 4 );
}
void ISR5 ( void )
{
	myPins.UpdateSignalInfo ( 5 );
}
void ISR6 ( void )
{
	myPins.UpdateSignalInfo ( 6 );
}
void ISR7 ( void )
{
	myPins.UpdateSignalInfo ( 7 );
}
void ISR8 ( void )
{
	myPins.UpdateSignalInfo ( 8 );
}
void ISR9 ( void )
{
	myPins.UpdateSignalInfo ( 9 );
}
void ISR10 ( void )
{
	myPins.UpdateSignalInfo ( 10 );
}
void ISR11 ( void )
{
	myPins.UpdateSignalInfo ( 11 );
}
void ISR12 ( void )
{
	myPins.UpdateSignalInfo ( 12 );
}
void ISR13 ( void )
{
	myPins.UpdateSignalInfo ( 13 );
}
void ISR14 ( void )
{
	myPins.UpdateSignalInfo ( 14 );
}
void ISR15 ( void )
{
	myPins.UpdateSignalInfo ( 15 );
}
void ISR16 ( void )
{
	myPins.UpdateSignalInfo ( 16 );
}
void ISR17 ( void )
{
	myPins.UpdateSignalInfo ( 17 );
}
void ISR18 ( void )
{
	myPins.UpdateSignalInfo ( 18 );
}
void ISR19 ( void )
{
	myPins.UpdateSignalInfo ( 19 );
}
void ISR20 ( void )
{
	myPins.UpdateSignalInfo ( 20 );
}
void ISR21 ( void )
{
	myPins.UpdateSignalInfo ( 21 );
}


