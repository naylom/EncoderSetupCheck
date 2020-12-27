/*
   Encoder setup check sketch,  Author Mark Naylor, 2020

   Arduino sketch to help check the inputs are being seen correctly before using the encoder quality check sketch

   First identifies which of first 21 digital pins will support interrupts then looks (for 15 seconds) for signals with pins in INPUT_PULLUP mode 
   
   and then checks any remaining pins (for another 15 seconds) using INPUT if any signals subsequently seen. Stops after 3 or more pins seen genertaing interrupts

   It uses interrupts to process in the incoming signals and hence requires an Arduino with 3 pins that can generate interrupts. All testing has been done on an Arduino Mega2560.

   Ver 1 - Initial release

 */

// Configuration items
#define BAUD_RATE                 115200
#define VER                       1
#define CHECK_TIME				  15						// time to wait (in seconds) to see signals 

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
	int				m_iCurrentPinMode;
	int				m_iPinNum;
	bool			m_bHasBeenSignalled;
	unsigned long	m_ulInterruptCount;
	unsigned long	m_ulLastSignalTime;
	int				m_iInputModeWhenSignalled;
	void			(*m_pISR)();		// Interrupt routine for this Pin
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
	unsigned long InterruptTime () { return m_ulLastSignalTime; }
	void InterruptTime ( unsigned long ulTime ) { m_ulLastSignalTime = ulTime; }

	String GetSignalledInputMode ()
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

	unsigned long GetInterruptCount()
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
		InterruptTime ( micros() );
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
	int			m_iNumPins;			// Number of digital pins that support interrupts on this board
	CPinData *	m_aPinData[MAX_PINS_TO_CHECK];
	
	// Implementations
private:
public:

	CBoardPins()
	{
		m_iNumPins = 0;
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

	void UpdateSignalInfo ( int iIndex )
	{
		// Find matching Index
		for ( int i = 0 ; i < NumPins() ; i++ )
		{
			if ( m_aPinData [ i ]->PinNum() == iIndex )
			{
				// Set has signalled to true
				if ( micros() - m_aPinData [ i ]->InterruptTime() > 5UL )
				{				
					m_aPinData [ i ]->HasSignalled ( true );
					m_aPinData [ i ]->IncInterruptCount();
					m_aPinData [ i ]->InterruptTime ( micros() );
				}
			}
		}
	}
	
	// Getter and Setters
	int NumPins() const { return m_iNumPins; }

	int GetPinNumber ( int iIndex )
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

	int NumPinsSignalled ()
	{
		int iResult = 0;
		for ( int i = 0 ; i < NumPins() ; i++ )
		{
			if ( m_aPinData [ i ]->HasSignalled() == true )
			{
				iResult++;
			}
		}
		return iResult;
	}

	void StartISRs ()
	{
		for ( int i = 0 ; i < NumPins() ; i ++ )
		{
			if ( m_aPinData [ i ]->HasSignalled() == false )
			{
				m_aPinData [ i ]->StartISR();
			}			
		}
	}

	void StopISRs()
	{
		for ( int i = 0 ; i < NumPins() ; i ++ )
		{
			m_aPinData [ i ]->StopISR();
		}
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
				Serial.print ( " signalled when pin was set to mode : " );
				Serial.print ( m_aPinData [ i ]->GetSignalledInputMode () );
			}
			else
			{
				Serial.print ( " did not signal" );
			}
		    Serial.println();
 	 	}
		Serial.println ( "\nNB signals seen when in INPUT mode can be false positives and only should be considered accurate if the pin has an external pullup resistor attached" );
	}
};

CBoardPins myPins; 

void setup ()
{
	Serial.begin (BAUD_RATE);
	delay ( 1000 );
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
			// put in PULLUP mode
			Serial.println ();
			Serial.println ( "Checking INPUT_PULLUP mode" );

			myPins.StopISRs();
			myPins.SetPinsMode ( INPUT_PULLUP );
			myPins.StartISRs ();

			Serial.println ( "Please turn encoder" );
			delay ( 1000 );
			break;
		
		case CHECK_TIME:
			// put in input mode
			Serial.println ( "\nChecking INPUT mode" );
			
			myPins.StopISRs ();
			myPins.SetPinsMode ( INPUT );
			myPins.StartISRs ();

			Serial.println ( "Please continue to turn encoder" );
			delay ( 1000 );
			break;
		
		case CHECK_TIME * 2:
			// end of program
			Serial.println ();
			Serial.println ( "\nFinished testing" );
			myPins.DisplayResults ();
			TerminateProgram ( "Program ending" );
			break;

		default:
			// check if 3 pins have signalled
			Serial.print ( "." );
			if ( myPins.NumPinsSignalled () >= 3 )
			{
				myPins.DisplayResults ();
				TerminateProgram ( "3 pins found, terminating program");
			}
			delay ( 1000 );
			break;
	}
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


