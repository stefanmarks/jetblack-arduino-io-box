
using UnityEngine;
using System;
using System.IO;
using System.IO.Ports;
using System.Collections;

/// <summary>
/// Class for communicating with the Arduino I/O module.
/// </summary>
/// 
public class ArduinoIO_Module : MonoBehaviour
{
	public String modulePort  = "COM3";
	public int    moduleSpeed = 115200;
	
	public int    hudUpdateInterval  = 500; // interval in ms for updating the HUD

	public int    ledLeft     = 7;
	public int    ledRight    = 3;
	public int    ledLCD      = 9;
	
	public int    buttonPollInterval = 100; // interval in ms for polling the buttons
	public int    buttonLeft         = 0;
	public int    buttonRight        = 1;
	
	public VehicleDataCollector    scriptVehicleData;
	public SimulationConfiguration scriptConfiguration;
	
	private const String CMD_ECHO = "E";
	
	
	/// <summary>
	/// Initialisation of the Arduino IO-Box script. 
	/// </summary>
	/// 
	public void Start() 
	{
		if ( !scriptVehicleData   ) Debug.LogError("No vehicle data collector script defined!");
		if ( !scriptConfiguration ) Debug.LogError("No simulation configuration script defined!");
		vehicleData = null;
		speedOfSound = scriptConfiguration.GetSpeedOfSound();
		
		serialPort = new SerialPort(modulePort, moduleSpeed, Parity.None, 8, StopBits.One);
		serialPort.Handshake   = Handshake.None;
		serialPort.RtsEnable   = false;
		serialPort.DtrEnable   = false;  // Disable DTR so NOT to reset Arduino board when connecting
		serialPort.ReadTimeout = 250;    // longer read timeout (module might have had a reset nevertheless)
		
		bool success = false;
		try
		{
			serialPort.Open();
		}
		catch (IOException)
		{
			serialPort = null;
		}
		
		if ( (serialPort != null) && serialPort.IsOpen )
		{
			int repeats = 8; // try several times to connect
			while ( (repeats-- > 0) && !success )
			{
				// send the ECHO command
				serialPort.WriteLine(CMD_ECHO);
				try
				{
					String serialNo = serialPort.ReadLine();
					if ( serialNo.Length > 1 )
					{
						Debug.Log ("Opened serial port " + modulePort + " to Arduino IO (Version: " + serialNo + ")");
						success  = true;
						serialPort.ReadTimeout = 50; // from now on, shorter response times, please
					    StartCoroutine(RunDiagnose());
					}
				}
				catch (TimeoutException)
				{
					// ignore
				}
			}
		}
		
		if ( !success )
		{
			Debug.LogError("Could not open serial port " + modulePort + " to the Arduino IO module.");
			if ( serialPort != null )
			{
				serialPort.Close();
				serialPort = null;
			}
		}
	}

	
	/// <summary>
	/// Runs a quick diagnose routine on the IO box.
	/// </summary>
	/// <returns>
	/// ???
	/// </returns>
	private IEnumerator RunDiagnose()
	{
		hudPage = HudPage.DIAGNOSE;
		const float stepWait = 0.25f;
		
		setLedColour(ledLCD, Color.yellow); setLed (ledLCD, 255, 0, 0);
		setText(0, "System Check:   ");
		yield return new WaitForSeconds(0.1f); // TODO: remove when IO setText works better
		setText(1, "                ");	
		yield return new WaitForSeconds(stepWait);
		
		getButtonPresses(buttonLeft); // clear button presses
		getButtonPresses(buttonRight);
		
		setLed(ledLeft, 100, 0, 0);       setLed(ledRight, 100, 0, 0);
		setLedColour(ledLeft, Color.red); setLedColour(ledRight, Color.red);
		setText(1, "HUD");	
		yield return new WaitForSeconds(stepWait);
		
		setLedColour(ledLeft, Color.green); setLedColour(ledRight, Color.green);
		setText(1, "Engine");	
		yield return new WaitForSeconds(stepWait);

		setLedColour(ledLeft, Color.blue); setLedColour(ledRight, Color.blue);
		setText(1, "Booster");	
		yield return new WaitForSeconds(stepWait);

		setLedColour(ledLeft, Color.yellow); setLedColour(ledRight, Color.yellow);
		setText(1, "Steering");	
		yield return new WaitForSeconds(stepWait);
		
		setLedColour(ledLeft, Color.cyan); setLedColour(ledRight, Color.cyan);
		setText(1, "Brakes  ");	
		yield return new WaitForSeconds(stepWait);

		setLedColour(ledLeft, Color.magenta); setLedColour(ledRight, Color.magenta);
		setText(1, "Parachute");	
		yield return new WaitForSeconds(stepWait);

		setLedColour(ledLeft, Color.white); setLedColour(ledRight, Color.white);
		setLedColour(9, Color.green);
		setText(0, " All Systems OK ");
		yield return new WaitForSeconds(0.1f); // TODO: remove when IO setText works better
		setText(1, " Ready to go... ");	

		yield return new WaitForSeconds(stepWait);
		setLed(ledLeft, 0); setLed(ledRight, 0);
		
		hudPage = HudPage.STANDBY;
	}
	
	
	/// <summary>
	/// Checks if the module is connected. 
	/// </summary>
	/// <returns>
	/// <code>true</code> if the module is connected, 
	/// <code>false</code> if not
	/// </returns>
	/// 
	public bool IsConnected()
	{
		return (serialPort != null);	
	}	
			

	/// <summary>
	/// The script is destroyed (e.g., when the game stops).
	/// </summary>
	/// 
	public void OnDestroy()
	{
		if ( IsConnected() )
		{
			// turn off LEDs and clear display
			setLed(ledLeft, 0, 0, 0); 
			setLed(ledRight, 0, 0, 0);
			setLed(ledLCD, 0, 0, 0);
			clearText();
			
			serialPort.Close();
			serialPort = null;
			// Debug.Log ("Serial port " + modulePort + " closed.");
		}
	}
	
	
	/// <summary>
	/// Loop for polling the inputs regularly and sending any state changes.
	/// </summary>
	/// 
	public void Update() 
	{
		if ( !IsConnected() || !scriptVehicleData ) return;
		if ( hudPage < HudPage.STANDBY ) return;
		
		checkVehicleState();
		checkButtons();
		updateHud();
	}
	
	/// <summary>
	/// Checks the state of the vehicle and flashes the warning LEDs if necessary.
	/// </summary>
	/// 
	private void checkVehicleState()
	{
		scriptVehicleData.GetData(ref vehicleData);
		VehicleSafetyControl.State state = vehicleData.safetyState;
		// any changes?
		if ( state != oldState )
		{
			switch ( state )
			{
				case VehicleSafetyControl.State.NOMINAL:
				{
					// no more blinking
					setLed(ledLeft, 0); setLed(ledRight, 0);
					break;
				}
				case VehicleSafetyControl.State.OVERHEAT_WHEEL_L:
				{
					// left LED blink
					setLedColour(ledLeft, scriptConfiguration.warningLightColour);
					setLed(ledLeft, 99, 500, 50);
					break;
				}
				case VehicleSafetyControl.State.OVERHEAT_WHEEL_R:
				{
					// right LED blink
					setLedColour(ledRight, scriptConfiguration.warningLightColour);
					setLed(ledRight, 99, 500, 50);
					break;
				}
				case VehicleSafetyControl.State.ABORT:
				{
					changeHudPage(HudPage.ABORT);					
					break;
				}
			}
			oldState = state;
		}	
	}
	
	/// <summary>
	/// Checks the buttons and updates the LCD display accordingly
	/// </summary>
	/// 
	private void checkButtons()
	{
		if ( IsConnected() && (Time.time > nextButtonPollTime) )
		{
			// poll the buttons and switch the HUD accordingly
			HudPage newPage = hudPage;
			if ( getButtonPresses(buttonLeft)  > 0 ) newPage--;
			if ( getButtonPresses(buttonRight) > 0 ) newPage++;
			
			// if vehicle starts to move: switch to speed page automatically
			if ( (vehicleData.speed > 0.1) && (hudPage == HudPage.STANDBY) )
			{
				newPage = HudPage.SPEED_KMH;
			}
			
			if ( newPage != hudPage )
			{
				// page has changed: stay within "selectable" range
				if ( newPage <= HudPage.FIRST_SELECTABLE ) 
				{
					newPage = HudPage.LAST_SELECTABLE - 1;
				}
				if ( newPage >= HudPage.LAST_SELECTABLE ) 
				{
					newPage = HudPage.FIRST_SELECTABLE + 1;
				}
				changeHudPage(newPage);
			}
			
			nextButtonPollTime = Time.time + (buttonPollInterval / 1000.0);
		}
	}
	
	/// <summary>
	/// Updates the data on the HUD.
	/// </summary>
	/// 
	private void updateHud()
	{
		if ( IsConnected() && (Time.time > nextHudUpdateTime) )
		{
			switch ( hudPage )
			{
				case HudPage.SPEED_KMH:
				{
					double speedKmH = vehicleData.speed * 3.6;
					String speed = speedKmH.ToString("0000");
					setText(0, 7, speed);
					break;
				}
				case HudPage.SPEED_MACH:
				{
					double speedMach = vehicleData.speed / speedOfSound;
					String speed = speedMach.ToString("0.00");
					setText(0, 7, speed);
					break;
				}
				case HudPage.FUEL1:
				{
				    double fuelPercent = vehicleData.engine1FuelLevel * 100.0;
					String fuel = fuelPercent.ToString("000");
					setText(0, 9, fuel);
					break;
				}
				case HudPage.FUEL2:
				{
				    double fuelPercent = vehicleData.engine2FuelLevel * 100.0;
					String fuel = fuelPercent.ToString("000");
					setText(0, 9, fuel);
					break;
				}
			}
			nextHudUpdateTime = Time.time + (hudUpdateInterval / 1000.0);
		}			
	}
	
	
	/// <summary>
	/// Changes the current HUD page.
	/// </summary>
	/// <param name='page'>
	/// the new HUD 
	/// </param>
	/// 
	private void changeHudPage(HudPage page)
	{
		if ( page != hudPage )
		{
			clearText();
			switch ( page )
			{
				case HudPage.SPEED_KMH:
				{
					setLedColour(ledLCD, Color.white); 
					setLed(ledLCD, 99, 0, 0);
					setText(0, "Speed: 0000 km/h");
					break;
				}
				case HudPage.SPEED_MACH:
				{
					setLedColour(ledLCD, Color.white); 
					setLed(ledLCD, 99, 0, 0);
					setText(0, "Speed: 0.00 mach");
					break;
				}
				case HudPage.FUEL1:
				{
					setLedColour(ledLCD, Color.white); 
					setLed(ledLCD, 99, 0, 0);
					setText(0, "Fuel E1: 000%");
					break;
				}
				case HudPage.FUEL2:
				{
					setLedColour(ledLCD, Color.white); 
					setLed(ledLCD, 99, 0, 0);
					setText(0, "Fuel E2: 000%");
					break;
				}
				case HudPage.ABORT:
				{
					// make background LED blink in red
					setLedColour(ledLCD, Color.red); 
					setLed (9, 99, 500, 50);
					// write text
					setText(0, "!!!! ABORT !!!! ");
					break;
				}
			}
			hudPage = page;
			
			// TODO:
			// don't immediately update the data
			// ...this has to go at some point...
			nextHudUpdateTime  = Time.time + 0.25;
			nextButtonPollTime = nextHudUpdateTime;
		}
	}
	
	/// <summary>
	/// Sets the brightness of a LED.
	/// </summary>
	/// <param name='led'>
	/// the number of the LED to set
	/// </param>
	/// <param name='brightness'>
	/// the brightness of the LED (0-99)
	/// </param>
	/// 
	private void setLed(int led, int brightness)
	{
		sendCommand("L" + led + "," + brightness);
	}

	/// <summary>
	/// Sets the brightness and blink interval of a LED.
	/// </summary>
	/// <param name='led'>
	/// the number of the LED to set
	/// </param>
	/// <param name='brightness'>
	/// the brightness of the LED (0-99)
	/// </param>
	/// <param name='interval'>
	/// the blink interval in milliseconds
	/// </param>
	/// <param name='ratio'>
	/// the blink ratio in percent
	/// </param>
	/// 
	private void setLed(int led, int brightness, int interval, int ratio)
	{
		sendCommand("L" + led + "," + brightness + "," + interval + "," + ratio);
	}
	
	/// <summary>
	/// Sets the colour of a LED.
	/// </summary>
	/// <param name='led'>
	/// the number of the LED to set
	/// </param>
	/// <param name='colour'>
	/// the colour of the LED
	/// </param>
	/// 
	private void setLedColour(int led, Color colour)
	{
		sendCommand("M" + led + 
			        "," + (int) (colour.r * 100) + 
			        "," + (int) (colour.g * 100) + 
			        "," + (int) (colour.b * 100));
	}
	
	/// <summary>
	/// Sets the text in the LCD.
	/// </summary>
	/// <param name='line'>
	/// the line where to change the text
	/// </param>
	/// <param name='text'>
	/// the text to write
	/// </param>
	/// 
	private void setText(int line, String text)
	{
		sendCommand("P" + line);
		sendCommand("T\"" + text + "\""); 
	}
	
	/// <summary>
	/// Sets the text in the LCD.
	/// </summary>
	/// <param name='line'>
	/// the line where to change the text
	/// </param>
	/// <param name='column'>
	/// the column where to change the text
	/// </param>
	/// <param name='text'>
	/// the text to write
	/// </param>
	/// 
	private void setText(int line, int column, String text)
	{
		sendCommand("P" + line + "," + column);
		sendCommand("T\"" + text + "\""); 
	}
	
	/// <summary>
	/// Clears the text.
	/// </summary>
	/// 
	private void clearText()
	{
		sendCommand("C");
	}
	
	/// <summary>
	/// Gets the number of button presses.
	/// </summary>
	/// <returns>
	/// the number of button presses since the last poll
	/// </returns>
	/// <param name='button'>
	/// the number of the button
	/// </param>
	/// 
	private int getButtonPresses(int button)
	{
		int numPresses = 0;
		String answer = sendRequest("b" + button);
		if ( answer.Length == 2 )
		{
			numPresses = (int) (answer[1] - '0');
		}
		return numPresses;
	}
	
	/// <summary>
	/// Sends a command and waits for the acknowledge char.
	/// </summary>
	/// <param name='command'>
	/// the command to send
	/// </param>
	/// 
	private void sendCommand(String command)
	{
		if ( IsConnected() )
		{
			// Debug.Log("send: " + command);
			serialPort.DiscardInBuffer();
			serialPort.WriteLine(command);
			try
			{
				String response = serialPort.ReadLine();
				if ( response != "+" )
				{
					Debug.LogError("Arduino IO box error for command " + command + ": " + response);
				}
			}
			catch (TimeoutException)
			{
				Debug.LogError("Arduino IO box timeout for command " + command);
			}
		}
	}
	
	/// <summary>
	/// Sends a command and waits for an answer.
	/// </summary>
	/// <returns>
	/// The answer of the request or "" if a timeout occured
	/// </returns>
	/// <param name='command'>
	/// the command to send
	/// </param>
	/// 
	private String sendRequest(String command)
	{
		String answer = "";
		if ( IsConnected() )
		{
			// Debug.Log("send: " + command);
			serialPort.DiscardInBuffer();
			serialPort.WriteLine(command);
			try
			{
				answer = serialPort.ReadLine();
				if ( answer == "!" )
				{
					Debug.LogError("Arduino IO box error for request " + command);
				}
			}
			catch (TimeoutException)
			{
				Debug.LogError("Arduino IO box timeout for request " + command);
			}
		}
		return answer;
	}
	
	
	private enum HudPage {
		DIAGNOSE = 0,
		STANDBY, 
		FIRST_SELECTABLE,
		SPEED_KMH, 
		SPEED_MACH, 
		FUEL1,
		FUEL2,
		LAST_SELECTABLE,
		ABORT
	};
	
	private SerialPort                 serialPort  = null;
	private VehicleData                vehicleData = null;
	private VehicleSafetyControl.State oldState;
		
	private double   nextHudUpdateTime  = 0;
	private double   nextButtonPollTime = 0;
	private HudPage  hudPage            = HudPage.DIAGNOSE;
	
	private double   speedOfSound;
}
