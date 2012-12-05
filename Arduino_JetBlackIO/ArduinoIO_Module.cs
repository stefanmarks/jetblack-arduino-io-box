
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
	
	public int    ledLeft     = 7;
	public int    ledRight    = 3;
	
	public VehicleDataCollector scriptVehicleData;
	
	private const String CMD_ECHO = "E";
	
	
	/// <summary>
	/// Initialisation of the Arduino IO-Box script. 
	/// </summary>
	/// 
	public void Start() 
	{
		if ( !scriptVehicleData ) Debug.LogError("No vehicle data collector script defined!");
		vehicleData = null;

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
						doUpdate = false;
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
		const float stepWait = 0.25f;
		
		setText(0, "System Check:   ");
		setText(1, "                ");	
		
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
		setText(0, " All Systems OK ");
		setText(1, " Ready to go... ");	

		yield return new WaitForSeconds(stepWait);
		setLed(ledLeft, 0); setLed(ledRight, 0);
		
		doUpdate = true; // now we can start running the real update routine
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
			setLed(ledLeft, 0); setLed(ledRight, 0);
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
		if ( !IsConnected() || !scriptVehicleData || !doUpdate ) return;
		
		// check vehicle state
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
					setLed(ledLeft, 99, 500, 50);
					break;
				}
				case VehicleSafetyControl.State.OVERHEAT_WHEEL_R:
				{
					// right LED blink
					setLed(ledRight, 99, 500, 50);
					break;
				}
				case VehicleSafetyControl.State.ABORT:
				{
					// turn off LEDs
					setLed(ledLeft, 0); setLed(ledRight, 0);
					// write text
					setText(0, "!!!! ABORT !!!! ");
					setText(1, "!!!! ABORT !!!! ");
					break;
				}
			}
			oldState = state;
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
		sendCommand("T\"" + text + "\"", 50 + 5 * text.Length); // variable timeout
	}
	
	/// <summary>
	/// Clears the text.
	/// </summary>
	/// 
	private void clearText()
	{
		setText(0, "                ");
		setText(1, "                ");
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
		sendCommand(command, 50);
	}
	
	/// <summary>
	/// Sends a command and waits for the acknowledge char.
	/// </summary>
	/// <param name='command'>
	/// the command to send
	/// </param>
	/// <param name='timeout'>
	/// the timeout in ms for this command
	/// </param>
	/// 
	private void sendCommand(String command, int timeout)
	{
		if ( IsConnected() )
		{
			// Debug.Log("send: " + command);
			serialPort.WriteLine(command);
			serialPort.ReadTimeout = timeout;
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
	
	private SerialPort                 serialPort  = null;
	private VehicleData                vehicleData = null;
	private VehicleSafetyControl.State oldState;
	private bool                       doUpdate    = false;
}
