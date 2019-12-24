#pragma once
//#include "ofConstants.h"
//#include "ofMesh.h"
//#include "ofRectangle.h"
#include "ofMain.h"

#include "ofxIOThread.h"

#include "GoIO_DLL_interface.h"

#define OFX_GO_IO_DEFAULT_TIMEOUT SKIP_TIMEOUT_MS_DEFAULT
#define OFX_GO_IO_DEFAULT_NUM_MEASUREMENTS 1000
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
	
private:
	
};


std::ostream& operator<<(std::ostream& os, const ofxGoIODevice& dev){
	os << "Name: " << dev.name << " Vendor Id: " << dev.vendorId << " Product Id: " << dev.productId;
	return os;
}


struct ofxGoIODeviceCalibrationProfile{
	float coeff [3];
	unsigned char calPageIndex;
	char equationType = 0;
	std::string units;
};

class ofxGoIOMeasurement{
public:
	std::vector<int>data;
	double aquisitionTime; // since app started running in seconds
	double sampleTimeInterval; // time interval between samples in seconds
	
	void addToMesh(ofMesh& mesh, const ofRectangle& viewport);
	
};

std::ostream& operator<<(std::ostream& os, const ofxGoIOMeasurement& data){
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


std::ostream& operator<<(std::ostream& os, const ofxGoIODeviceCalibrationProfile& profile){
	os << "ofxGoIO Device Calibration Profile:" << std::endl;
	os << "    Page Index: " << profile.calPageIndex << std::endl;
	os << "    Coefficients: " << profile.coeff[0] << ", " << profile.coeff[1] << ", " << profile.coeff[2] << std::endl;
	os << "    Equation type: " << profile.equationType;
	if (profile.units.size()){
		os  << std::endl << " ( " << profile.units << " )";
	}
	return os;
}

class ofxGoIO: public ofxIOThread{
public:
	ofxGoIO();
	~ofxGoIO();
	
	
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
	static std::string stateToString(State s){
		switch(s){
			case OFX_GO_IO_STATE_NOT_SETUP: return "Not Setup";
			case OFX_GO_IO_STATE_SETUP: return "Setup";
			case OFX_GO_IO_STATE_MEASURING: return "Measuring";
			case OFX_GO_IO_STATE_SETTING_INTERVAL: return "Setting Interval";
			case OFX_GO_IO_STATE_CALIBRATING: return "Calibrating";
		}
		return "";
	}

	const ofxGoIODevice& getCurrentDevice();
	
	State getState();
	
	void draw();
	
	ofEvent<ofxGoIOMeasurement>newMeasurementEvent;
	
protected:
	virtual bool shouldRepeatWithDelay(uint64_t& delay) override;
	void threadedFunction();
	
	
	void updateCalibration();
	void updateMeasurements();



	ofEventListener exitListener;
	
	
private:
	static void init();
	static void uninit();
	static bool& isInited();

	void setState(State newState);
	
	std::atomic<State> state;

	std::atomic<double> measurementsInterval; // interval in seconds
	
	std::atomic<bool> bHasNewData;
	
	GOIO_SENSOR_HANDLE handle = NULL;
	
	ofxGoIODevice currentDevice;
	ofxGoIODeviceCalibrationProfile currentDeviceCalibration;
	
	ofxGoIODeviceCalibrationData calibrationData;
	
	State stateBeforeCalibration;
	
	std::mutex dataMutex;
	

	ofxGoIOMeasurement lastMeasurement;
	
	std::vector<ofxGoIOMeasurement> measurements;

	size_t currentMeasurementIndex;
	
};