using UnityEngine;
using System.Collections;
using System.IO.Ports;
using System.Threading;

public class ArduinoLCD : MonoBehaviour 
{
    //Setup parameters to connect to Arduino
    public static SerialPort sp = new SerialPort("COM4", 9600);//, Parity.None, 8, StopBits.One);
    public string strIn;  
	public string message;
	public int selection = 0;
	private VehicleDataCollector collector;
	private VehicleData          vehicleData;
	
    // Use this for initialization
    void Start () 
    {
		print("Starting Arduino");
		
    	OpenConnection();
		// get the vehicle data collector
		collector = GetComponent<VehicleDataCollector>();
		if ( !collector ) print("Could not find the vehicle data collector component");
		
		
		InvokeRepeating("FixedUpdate2", 0f, 0.5f);
		sp.Write("*");
    }
	
	void FixedUpdate2()
	{
		collector.GetData(ref vehicleData);
            //Read incoming data
		try{
       	strIn = sp.ReadLine();	
			//print ("strIn: " + strIn);
		selection = System.Int32.Parse(strIn);
			//print("Found: " + selection);
		}catch(System.Exception)
		{
			
		}
		try
		{
		if(selection != -1)
		{
			switch(selection)
			{
				case 0:
				double speed = vehicleData.speed * 3.6;
				//write the new text values to the LCD, # is to move the cursor to the next line
				sp.Write("Speed:#" + speed.ToString("000 ") + "km/h"); 
				break;
				
			case 1:
				sp.Write("Parachute:#" + (vehicleData.parachuteDeployed ? "deployed" : "standby")); 
				break;
				
			case 2:
				sp.Write ("Fuel 1:#" + vehicleData.engine1FuelLevel.ToString("000%"));
				break;
				
			case 3:
				sp.Write("Fuel 2:#" + vehicleData.engine2FuelLevel.ToString("000%"));
				break;
				
			case 4:
				sp.Write("Pedal Brk:#" + vehicleData.pedalBrake.ToString("000%"));
				break;
				
			case 5:
				sp.Write("Pedal Acc:#" + vehicleData.pedalThrust.ToString("000%"));
				break;
				
			default:
				break;
			}
		}
           
	}catch(System.Exception){}
	}

    void Update()
    {
		

    }

    //Function connecting to Arduino
    public void OpenConnection() 
    {
       if (sp != null) 
       {
         if (sp.IsOpen) 
         {
          sp.Close();
          print("Closing port, because it was already open!");
         }
         else 
         {
		  try{
          sp.Open();  // opens the connection
		  sp.DtrEnable = true;
          sp.ReadTimeout = 50;  // sets the timeout value before reporting error
          print("Port Opened!");
				} 
				catch(System.IO.IOException)
				{
					Debug.LogError("Arduino was not found on COM4");	
				}
         }
       }
       else 
       {
         if (sp.IsOpen)
         {
          print("Port is already open");
         }
         else 
         {
          print("Port == null");
         }
       }
    }

    void OnApplicationQuit() 
    {
		sp.Write(" Finished");
       sp.Close();
    }
}