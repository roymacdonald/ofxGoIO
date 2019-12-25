#pragma once
//#include "ofConstants.h"
//#include "ofMesh.h"
//#include "ofRectangle.h"
#include "ofMain.h"

//#define OFX_GO_IO_USE_THREAD
#ifdef OFX_GO_IO_USE_THREAD
#include "ofxIOThread.h"
#endif

#include "GoIO_DLL_interface.h"

//
//IMPORTANTE. Todas las funciones de GOIO_ se tienen que llamar desde el mismo thread the abrio el device, esto no esta resuelto y hay qye resolverlo.
//Quizas hacer una version no threaded no es mala idea. Usando unos ifdef

#define OFX_GO_IO_DEFAULT_TIMEOUT SKIP_TIMEOUT_MS_DEFAULT
#define OFX_GO_IO_DEFAULT_NUM_MEASUREMENTS 1000
//---------------------------------------------------------------------------------------------------
class ofxGoIODevice{
public:
	ofxGoIODevice(){}
	ofxGoIODevice(std::string deviceName, int vid, int pid) :
		name(deviceName),
		vendorId(vid),
		productId(pid)
	{}
	
	std::string name;
	int vendorId = -1;		//nr id
	int productId = -1;		//USB product id
	bool isSet() const{ return !name.empty() && vendorId != -1 && productId != -1;}
	
	unsigned char id;
	string longName = "";
		
};

std::ostream& operator<<(std::ostream& os, const ofxGoIODevice& dev);
//---------------------------------------------------------------------------------------------------

struct ofxGoIODeviceCalibrationProfile{
	float coeff [3];
	unsigned char calPageIndex;
	char equationType = 0;
	std::string units;
};


std::ostream& operator<<(std::ostream& os, const ofxGoIODeviceCalibrationProfile& profile);

//---------------------------------------------------------------------------------------------------
class ofxGoIOMeasurement{
public:
	std::vector<int>data;
	double aquisitionTime; // since app started running in seconds
	double sampleTimeInterval; // time interval between samples in seconds
	
};

std::ostream& operator<<(std::ostream& os, const ofxGoIOMeasurement& data);
//---------------------------------------------------------------------------------------------------

class ofxGoIODeviceCalibrationData{
public:
	void setup(size_t _numSamples);
	size_t size();
	void clear();
	bool process(GOIO_SENSOR_HANDLE handle);

	size_t numSamples;
	size_t currentSample = 0;
	std::vector<int> rawMeasurements;
	std::vector<double> volts;
	std::vector<double> calbMeasurements;
	double averageCalbMeasurement = 0;
	
};

//---------------------------------------------------------------------------------------------------
class ofxGoIORenderer;

class ofxGoIO
#ifdef OFX_GO_IO_USE_THREAD
: public ofxIOThread
#endif
{
public:
	friend class ofxGoIORenderer;
	ofxGoIO();
	virtual ~ofxGoIO();
	enum State{
		OFX_GO_IO_STATE_NOT_SETUP = 0,
		OFX_GO_IO_STATE_SETUP,
		OFX_GO_IO_STATE_MEASURING,
		OFX_GO_IO_STATE_SETTING_INTERVAL,
		OFX_GO_IO_STATE_CALIBRATING,
	};

	/// --------- STATIC FUNTIONS -----------
	static std::vector<ofxGoIODevice> getAvailableDevices();
	static std::string getDevicesAsString(const std::vector<ofxGoIODevice>& devices);
	static void printDevices(const std::vector<ofxGoIODevice>& devices);
	
	/// get the state as a string.
	/// assuming that you've declared `ofxGoIO goIO;` already.
	/// ````
	/// string stateStr = ofxGoIO::stateToString(goIO.getState());
	/// ````
	static std::string stateToString(State s);

	/// --------- MEMBER FUNTIONS -----------
	/// setup() needs to be called as soon as the ofxGoIO object is created, only once per object. If it is not called then it will fail at comminicating with the desired device.
	/// When no arguments are passed the first available device is chosen.
	/// will return true if successfully connected to a device.
	bool setup();
	
	/// When passing an ofxGoIODevice object as argument it will look for this object in the list of available ones and connect to it if it is available. You can get this object from calling ofxGoIO::getAvailableDevices(); in the following way
	///
	/// ```
	/// auto devices = ofxGoIO::getAvailableDevices();
	/// // Assuming that you have already declare `ofxGoIO goIO;`
	/// size_t deviceIndex = 1;// choose the adecuate index
	/// ofxGoIO::printDevices(devices); // you can print the devices in order to determine the right index
	/// if(deviceIndex < devices.size()){// make sure there is an available device and the desired index is in range
	/// 	goIO.setup(devices[deviceIndex]);
	/// }
	/// ```
	/// will return true if successfully connected to the desired device.
	bool setup(ofxGoIODevice device);
	
	/// setup by passing the name, vendor id and product id of the device you want to connect to.
	/// If you want to connect to a device of which you already know this values use this
	/// will return true if successfully connected to the desired device.
	bool setup(std::string deviceName, int vendorId, int productId);

	/// startMeasurements(...)
	/// Send a coomand to the device so it starts sending measurements to the computer
	///
	/// By passing no arguments to this function the default timeout is used.
	/// If an argument is passed it corresponds to the timeout in milliseconds, which is the amount of time to wait in order to get an response from the device before giving up.
	/// returns true if command was successfull, false otherwise.
	/// NOTE: This function is not called automatically from setup(), so you must call it after calling setup in order to get measurements
	bool startMeasurements();
	bool startMeasurements(int timeoutMs);

	/// stopMeasurements(...)
	/// arguments and retrurn values behave exactly the same as startMeasurements(...)
	bool stopMeasurements();
	bool stopMeasurements(int timeoutMs);
	

	
	/// getMeasurementTickInSeconds()
	///		The measurement period for Go! devices is specified in discrete 'ticks', so the actual time between
	///		measurements is an integer multiple of the tick time. The length of time between ticks is different
	///		for Go! Link versus Go! Temp.
	///
	///	Return:		If not initializd properly, then this routine returns -1.0, else the return value = the length of time
	///				in seconds between ticks.
	double getMeasurementTickInSeconds();
	
	
	
	/// get min and max measurement period functions
	
	///	Return:		If not initializd properly, then this routine returns -1.0, else the return value = minimum or maximum measurement
	///				period in seconds that is supported by the device.
	
	double getMinimumMeasurementPeriod();
	double getMaximumMeasurementPeriod();
	
	
	
/// setMeasurementPeriod(...)
///		Purpose:	Set the device's measurement period to a specified number of seconds. The Go! sensor will report measurements
///					to the computer at the measurement period interval once measurements have been started. These
///					measurements are held in the GoIO Measurement Buffer.
///
///					Because the measurement period is constrained to be a multiple of the tick returned by
///					getMeasurementTickInSeconds(), and because it must be
///						>= getMinimumMeasurementPeriod(), and
///						<= getMaximumMeasurementPeriod(),
///					the actual period is different than the desiredPeriod.
///
///					You can determine the actual period by calling getMeasurementPeriod().
///
///					If the device was taking measurements, these will get paused, the measurement period will be set, and measurements will be resumed.
///
///		Return:		true ifsuccessful, else false
///

	bool   setMeasurementPeriod( double desiredPeriod, int timeoutMs = OFX_GO_IO_DEFAULT_TIMEOUT);
	
	/// getMeasurementPeriod(...)
	///
	///	Return:		1000000.0 if not successful, else the measurement period in seconds.
	double getMeasurementPeriod( int timeoutMs = OFX_GO_IO_DEFAULT_TIMEOUT);
	
	/// get the number of available measurements in the read buffer, which have not been read yet.
	/// These are read automatically in each update call, which happens automatically.
	/// You'll probably not need to use this function
	int    getNumMeasurementsAvailable();
	
	/// get the device's current calibration profile.
	const ofxGoIODeviceCalibrationProfile&  getCurrentCalibrationFromDevice();
	
	/// call this function to calibrate the device.
	/// Look at its documentation to find out what you need to do with it to calibrate.
	/// Once ready with the device, call this function, passing the ammount of reading you want to use to calibrate.
	/// If reading is active, it will be paused while calibrating and resumed once ready.
	/// calibrationEndEvent will be notified once calibration ends
	void calibrate( size_t numSamples);
	
	// ofEvent triggered when the calibration routine has ended
	ofEvent<ofxGoIODeviceCalibrationProfile> calibrationEndEvent;
	
	
	/// close the opened device
	void close();
	
	/// Return true if the call to setup() was successful
	bool isSetup();
	
	/// clear the interface read buffer.
	void clear();
	
	
	/// get the currently opened device
	const ofxGoIODevice& getCurrentDevice();
	
	/// get the current internal state
	State getState();
	
	
	/// Event triggered when a new set of measurement has arrived.
	ofEvent<ofxGoIOMeasurement>newMeasurementEvent;
	
protected:
	bool getDeviceIdAndLongName(ofxGoIODevice& dev);
	
	#ifdef OFX_GO_IO_USE_THREAD
	virtual bool shouldRepeatWithDelay(uint64_t& delay) override;
	void threadedFunction();
#endif
	
	bool openDevice(ofxGoIODevice device);
	bool openDevice(std::string _deviceName, int vid, int pid);
	
	void update();
	void updateCalibration();
	void updateMeasurements();


private:

	ofEventListener exitListener;
#ifndef OFX_GO_IO_USE_THREAD
	ofEventListener updateListener;
#endif
	
	static void init();
	static void uninit();
	static bool& isInited();

	void setState(State newState);
	
#ifdef OFX_GO_IO_USE_THREAD
	std::atomic<State> state;
	std::atomic<double> measurementsInterval; // interval in seconds
	std::atomic<bool> bHasNewData;
	std::mutex dataMutex;
#else
	State state;
	double measurementsInterval; // interval in seconds
	bool bHasNewData;
#endif
	
	
	
	GOIO_SENSOR_HANDLE handle = NULL;
	
	ofxGoIODevice currentDevice;
	ofxGoIODeviceCalibrationProfile currentDeviceCalibration;
	
	ofxGoIODeviceCalibrationData calibrationData;
	
	State stateBeforeCalibration;
	
	
	

	ofxGoIOMeasurement lastMeasurement;
	
	std::vector<ofxGoIOMeasurement> measurements;

	size_t currentMeasurementIndex;
	
};
