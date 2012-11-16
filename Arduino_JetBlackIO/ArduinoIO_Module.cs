
using UnityEngine;
using System;
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
	
    private const String CMD_ECHO = "E";
	
	
	/// <summary>
	/// Initialisation of the class. 
	/// </summary>
	/// 
	public void Start() 
	{
		bool success = false;
		
		serialPort = new SerialPort(modulePort, moduleSpeed, Parity.None, 8, StopBits.One);
		serialPort.Handshake   = Handshake.None;
		serialPort.RtsEnable   = false;
		serialPort.DtrEnable   = false;  // Disable DTR so NOT to reset Arduino board when connecting
		serialPort.ReadTimeout = 250;    // longer read timeout (module might have had a reset nevertheless)
		
		serialPort.Open();
		if ( serialPort.IsOpen )
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
			serialPort.Close();
			serialPort = null;
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
	/// The script is disabled (e.g., when the game stops).
	/// The asynchronous receiving method is stopped
	/// by setting the module address to null.
	/// </summary>
	/// 
	public void OnApplicationQuit()
	{
		if ( serialPort != null )
		{
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
		if ( IsConnected() )
		{
			int i = (int) (50 + (49 * Math.Sin (Time.time * 4)));
			serialPort.WriteLine(String.Format("L0,{0}", i));
			i = (int) (50 + (49 * Math.Cos (Time.time * 4)));
			serialPort.WriteLine(String.Format("L1,{0}", i));
		}
	}
	
	
	private void PrintBuffer(String prefix, byte[] buffer, int length)
	{
		String strOut = prefix;
		for ( int i = 0 ; i < length ; i++ )
		{
			strOut += String.Format(" {0:X2}", buffer[i]);
			if ( (buffer[i] >= 32) && (buffer[i] < 128) )
			{
				strOut += ":" + (char) buffer[i];
			}
		}
		Debug.Log(strOut);
	}
	
	
	private SerialPort serialPort = null;
}
