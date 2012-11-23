
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
				// clean up stuff that might have been caused by initialisation
				serialPort.DiscardInBuffer();
				serialPort.DiscardOutBuffer();
			
				// send the ECHO command
				serialPort.WriteLine(CMD_ECHO);
				try
				{
					String serialNo = serialPort.ReadLine();
					if ( serialNo.Length > 1 )
					{
						Debug.Log ("Opened serial port " + modulePort + " to Arduino IO (Version: " + serialNo + ")");
						serialPort.ReadTimeout = 50;  // 50ms read timeout from now on
						serialPort.DiscardInBuffer(); // discard any crap that was sent, too
						serialPort.DiscardOutBuffer();
						success = true;
						
						setText(0, "  JetBlack HUD  ");
						setText(1, "      Ready     ");
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
			setLed(0, 0); setLed(1, 0);
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
					setLed(0, 0); setLed(1, 0);
					break;
				}
				case VehicleSafetyControl.State.OVERHEAT_WHEEL_L:
				{
					// left LED blink
					setLed(0, 99, 500, 50);
					break;
				}
				case VehicleSafetyControl.State.OVERHEAT_WHEEL_R:
				{
					// right LED blink
					setLed(1, 99, 500, 50);
					break;
				}
				case VehicleSafetyControl.State.ABORT:
				{
					// turn off LEDs
					setLed(0, 0); setLed (1, 0);
					// write text
					setText(0, "!!!! ABORT !!!! ");
					setText(1, "!!!! ABORT !!!! ");
					break;
				}
			}
			oldState = state;
		}	
	}
	
	
	private void setLed(int led, int brightness)
	{
		if ( IsConnected() )
		{
			serialPort.WriteLine("L" + led + "," + brightness);
		}
	}
	
	private void setLed(int led, int brightness, int interval, int ratio)
	{
		if ( IsConnected() )
		{
			serialPort.WriteLine("L" + led + "," + brightness + "," + interval + "," + ratio);
		}
	}
	
	private void setText(int line, String text)
	{
		if ( IsConnected() )
		{
			serialPort.WriteLine("T" + line + ",0,\"" + text + "\"");
		}
	}

	private void clearText()
	{
		setText(0, "                ");
		setText(1, "                ");
	}
	
	private SerialPort                 serialPort  = null;
	private VehicleData                vehicleData = null;
	private VehicleSafetyControl.State oldState;
}
