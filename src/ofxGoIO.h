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
	
private:
	
};


inline std::ostream& operator<<(std::ostream& os, const ofxGoIODevice& dev){
	os << "Name: " << dev.name << std::endl;
	os << "Vendor Id: " << dev.vendorId << std::endl;
	os << "Product Id: " << dev.productId << std::endl;
	os << "Device Id: " << dev.id << std::endl;
	if(dev.longName.size()){
		os << "Long Name: " << dev.longName << std::endl;
	}
	return os;
}
//---------------------------------------------------------------------------------------------------

struct ofxGoIODeviceCalibrationProfile{
	float coeff [3];
	unsigned char calPageIndex;
	char equationType = 0;
	std::string units;
};


inline std::ostream& operator<<(std::ostream& os, const ofxGoIODeviceCalibrationProfile& profile){
	os << "ofxGoIO Device Calibration Profile:" << std::endl;
	os << "    Page Index: " << profile.calPageIndex << std::endl;
	os << "    Coefficients: " << profile.coeff[0] << ", " << profile.coeff[1] << ", " << profile.coeff[2] << std::endl;
	os << "    Equation type: " << profile.equationType;
	if (profile.units.size()){
		os  << std::endl << " ( " << profile.units << " )";
	}
	return os;
}

//---------------------------------------------------------------------------------------------------
class ofxGoIOMeasurement{
public:
	std::vector<int>data;
	double aquisitionTime; // since app started running in seconds
	double sampleTimeInterval; // time interval between samples in seconds
	
	void addToMesh(ofMesh& mesh, const ofRectangle& viewport);
	
};

inline std::ostream& operator<<(std::ostream& os, const ofxGoIOMeasurement& data){
	os << "Measurement: " << std::endl;
	os << "     Num Samples:  " << data.data.size() << std::endl;
 	os << "     Aquisition time:  " << data.aquisitionTime << std::endl;
	os << "     sampleTimeInterval:  " << data.sampleTimeInterval << std::endl;
	for(auto&d: data.data){
		os << d << ", ";
	}
	os << std::endl;
	return os;
}

//---------------------------------------------------------------------------------------------------

class ofxGoIODeviceCalibrationData{
public:
//	ofxGoIODeviceCalibrationData(size_t _numSamples):
//		numSamples(_numSamples),
//		rawMeasurements (_numSamples),
//		volts (_numSamples),
//		calbMeasurements (_numSamples)
//	{}
	void setup(size_t _numSamples){
		numSamples = _numSamples;
		rawMeasurements.resize(_numSamples);
		volts.resize(_numSamples);
		calbMeasurements.resize(_numSamples);
		averageCalbMeasurement = 0;
		currentSample = 0;
	}
	size_t size() {return numSamples;}
	void clear(){
		numSamples = 0;
		rawMeasurements.clear();
		volts.clear();
		calbMeasurements.clear();
	}
	bool process(GOIO_SENSOR_HANDLE handle){
		if(handle && numSamples > 1){
			averageCalbMeasurement = 0;
			for (size_t i = 0; i < numSamples; i++){
				volts[i] = GoIO_Sensor_ConvertToVoltage(handle, rawMeasurements[i]);
				calbMeasurements[i] = GoIO_Sensor_CalibrateData(handle, volts[i]);
				averageCalbMeasurement += calbMeasurements[i];
			}
			averageCalbMeasurement = averageCalbMeasurement/numSamples;
			return true;
		}
		return false;
	}
//protected:
	size_t numSamples;
	size_t currentSample = 0;
	std::vector<int> rawMeasurements;
	std::vector<double> volts;
	std::vector<double> calbMeasurements;
	double averageCalbMeasurement = 0;
	
};

//---------------------------------------------------------------------------------------------------

class ofxGoIO
#ifdef OFX_GO_IO_USE_THREAD
: public ofxIOThread
#endif
{
public:
	ofxGoIO();
	virtual ~ofxGoIO();
	
	
	static std::vector<ofxGoIODevice> getAvailableDevices();
	static std::string getAvailableDevicesAsString();
	static void printAvailableDevices();
	
	void setup();
	void setup(ofxGoIODevice device);
	void setup(std::string deviceName, int vendorId, int productId);

	bool startMeasurements(int timeoutMs = OFX_GO_IO_DEFAULT_TIMEOUT);
	bool stopMeasurements(int timeoutMs = OFX_GO_IO_DEFAULT_TIMEOUT);
	

	bool openDevice(ofxGoIODevice device);
	bool openDevice(std::string _deviceName, int vid, int pid);



	
	
	double getMeasurementTickInSeconds();
	double getMinimumMeasurementPeriod();
	double getMaximumMeasurementPeriod();
	bool    setMeasurementPeriod( double desiredPeriod, int timeoutMs = OFX_GO_IO_DEFAULT_TIMEOUT);
	double getMeasurementPeriod( int timeoutMs = OFX_GO_IO_DEFAULT_TIMEOUT);
	int    getNumMeasurementsAvailable();
	
	
	const ofxGoIODeviceCalibrationProfile&  getCurrentCalibrationFromDevice();
	
	bool calibrate( size_t numSamples);
		
	void close();
	bool isSetup();
	
	void clear();
	
	enum State{
		OFX_GO_IO_STATE_NOT_SETUP = 0,
		OFX_GO_IO_STATE_SETUP,
		OFX_GO_IO_STATE_MEASURING,
		OFX_GO_IO_STATE_SETTING_INTERVAL,
		OFX_GO_IO_STATE_CALIBRATING,
	};
	static std::string stateToString(State s);
	
	const ofxGoIODevice& getCurrentDevice();
	
	State getState();
	
	void draw();
	
	ofEvent<ofxGoIOMeasurement>newMeasurementEvent;
	
	bool getDeviceIdAndLongName(ofxGoIODevice& dev);
	
	
protected:
	#ifdef OFX_GO_IO_USE_THREAD
	virtual bool shouldRepeatWithDelay(uint64_t& delay) override;
	void threadedFunction();
#endif
	
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
