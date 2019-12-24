#include "ofxGoIO.h"
//#include "ofLog.h"
//#include "ofMath.h"
//
//#ifdef TARGET_WIN32
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
//#include <windows.h>
//#pragma warning(disable: 4996)
//#endif
//#ifdef TARGET_OSX
//#include <Carbon/Carbon.h>
//#endif
//

using namespace std;
//---------------------------------------------------------------------------------------------
ofxGoIO::ofxGoIO()
:ofxIOThread(std::bind(&ofxGoIO::threadedFunction, this))
{
	init();
	state = OFX_GO_IO_STATE_NOT_SETUP;
	
	measurementsInterval = 0.04;//40 milliseconds
	currentMeasurementIndex = 0;
	bHasNewData = false;
	exitListener = ofEvents().exit.newListener([&](ofEventArgs&){
		uninit();
	});
}
ofxGoIO::~ofxGoIO(){
	close();
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::init(){
	if(!isInited()){
		if(GoIO_Init() == 0){
			isInited() = true;
		}
	}else{
		ofLogVerbose("ofxGoIO::init()") << "already inited";
	}
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::uninit(){
	if(isInited()){
		if(GoIO_Uninit() == 0){
			isInited() = false;
		}
	}else{
		ofLogVerbose("ofxGoIO::uninit()") << "cant uninit as it was not inited";
	}
}
//---------------------------------------------------------------------------------------------
bool& ofxGoIO::isInited(){
	static unique_ptr<bool> i =  make_unique<bool>();
	return *i;
}
//---------------------------------------------------------------------------------------------
void addDevices(vector<ofxGoIODevice> & devices, int vid, int pid){
	char deviceName[GOIO_MAX_SIZE_DEVICE_NAME];
	int numDevs = GoIO_UpdateListOfAvailableDevices(vid, pid);
	for(int i = 0; i < numDevs; i ++){
		if(GoIO_GetNthAvailableDeviceName(deviceName, GOIO_MAX_SIZE_DEVICE_NAME, vid, pid, i) == 0){
			devices.push_back(ofxGoIODevice(deviceName, vid, pid));
		}
	}
}
//---------------------------------------------------------------------------------------------
vector<ofxGoIODevice> ofxGoIO::getAvailableDevices(){

	init();
	
	vector<ofxGoIODevice> devices;
	
	addDevices(devices, VERNIER_DEFAULT_VENDOR_ID, SKIP_DEFAULT_PRODUCT_ID);
	addDevices(devices, VERNIER_DEFAULT_VENDOR_ID, USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID);
	addDevices(devices, VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID);
	addDevices(devices, VERNIER_DEFAULT_VENDOR_ID, MINI_GC_DEFAULT_PRODUCT_ID);

	return devices;
	
}

//---------------------------------------------------------------------------------------------
string ofxGoIO:: getAvailableDevicesAsString(){
	stringstream ss;
	auto devs = getAvailableDevices();
	for(auto&d : devs){
		ss << d << endl;
	}
	return ss.str();
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::printAvailableDevices(){
	cout << "--------------------------"<< endl;
	cout << "ofxGoIO available devices:"<< endl;
	cout << getAvailableDevicesAsString() << endl;
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::setup(ofxGoIODevice _device){
	auto devs = getAvailableDevices();
	if(devs.size() == 0){
		ofLogError("ofxGoIO::setup") << "No devices available";
		return;
	}
	if(!_device.isSet()){
		if(openDevice(devs[0])){
			setState(OFX_GO_IO_STATE_SETUP);
		}
	}else{
		bool bDeviceFound = false;
		for(auto&d : devs){
			if(d.name == _device.name && d.vendorId == _device.vendorId && d.productId == _device.productId){
				bDeviceFound = true;
				break;
			}
		}
		if(bDeviceFound){
			if(openDevice(_device)){
				setState(OFX_GO_IO_STATE_SETUP);
			}
		}else{
			ofLogWarning("ofxGoIO::setup") << "failed. Device not found.";
		}
	}
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::setup(string deviceName, int vid, int pid){
	setup({deviceName, vid, pid});
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::setup(){
	setup(ofxGoIODevice());
}

//---------------------------------------------------------------------------------------------
bool ofxGoIO::openDevice( ofxGoIODevice device){
	
	
	if (handle)
	{
		char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		int openVendorId, openProductId;
		GoIO_Sensor_GetOpenDeviceName(handle, openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &openVendorId, &openProductId);

		if (0 != device.name.compare(string(openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME)))
		{
			//Close the open device if it does not match the new one.
			close();
		}else{
			ofLogVerbose( "ofxGoIO::openDevice") << "device " << device << "is already opened";
			return true;
		}
	}
	if (!handle){
		if(!device.isSet()){
			auto devs = getAvailableDevices();
			if(devs.size()){
				device = devs[0];
			}else{
				ofLogVerbose( "ofxGoIO::openDevice") << "could not auto assign a device as there are non available";
				return false;
			}
		}
		handle = GoIO_Sensor_Open(device.name.c_str(), device.vendorId, device.productId, 0);
	}
	if (handle)
	{
		this->currentDevice = device;
		
		unsigned char charId;
		GoIO_Sensor_DDSMem_GetSensorNumber(handle, &charId, 0, 0);
		int id = charId;
		
		cout << "Sensor id = " << id;

		char tmpstring[50];
		GoIO_Sensor_DDSMem_GetLongName(handle, tmpstring, sizeof(tmpstring));
		string longName = tmpstring;
		
		if (longName.size()){
			cout << " ( " << longName << " ) ";
		}
		cout << endl;
	
		cout << getCurrentCalibrationFromDevice() << endl;
		
	}else{
		ofLogError("ofxGoIO::openDevice") << "failed to open device " << device;
	}
	
	return (handle != NULL);
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::openDevice(string deviceName, int vid, int pid){
	return openDevice({deviceName, vid, pid});
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::close(){
	if(handle != NULL){
		GoIO_Sensor_Close(handle);
		handle = NULL;
	}
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::isSetup(){
	return state != OFX_GO_IO_STATE_NOT_SETUP;
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::clear(){
	if(handle != NULL){
		GoIO_Sensor_ClearIO(handle);
	}
}
//---------------------------------------------------------------------------------------------
double ofxGoIO::getMeasurementTickInSeconds(){
	auto ret = GoIO_Sensor_GetMeasurementTickInSeconds(	handle );
	if(ret > 0){
		ofLogWarning("ofxGoIO::getMeasurementTickInSeconds") << "failed";
	}
	return ret;
}
//---------------------------------------------------------------------------------------------
double ofxGoIO::getMinimumMeasurementPeriod(){
	auto ret = GoIO_Sensor_GetMinimumMeasurementPeriod(	handle );
	if(ret > 0){
		ofLogWarning("ofxGoIO::getMinimumMeasurementPeriod") << "failed";
	}
	return ret;
}
//---------------------------------------------------------------------------------------------
double ofxGoIO::getMaximumMeasurementPeriod(){
	auto ret = GoIO_Sensor_GetMaximumMeasurementPeriod(	handle );
	if(ret > 0){
		ofLogWarning("ofxGoIO::getMaximumMeasurementPeriod") << "failed";
	}
	return ret;
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::setMeasurementPeriod( double desiredPeriod, int timeoutMs){
	
	auto prevState = getState();
	if(prevState == OFX_GO_IO_STATE_MEASURING){
		stopMeasurements();
	}
	setState(OFX_GO_IO_STATE_SETTING_INTERVAL);
	bool bSuccess = GoIO_Sensor_SetMeasurementPeriod( handle, desiredPeriod, timeoutMs) == 0;
	if(bSuccess){
		getMeasurementPeriod(timeoutMs);
	}else{
		ofLogWarning("ofxGoIO::setMeasurementPeriod") << "failed";
	}
	if(prevState == OFX_GO_IO_STATE_MEASURING){
		startMeasurements();
	}else{
		setState(prevState);
	}
	return bSuccess;
}
//---------------------------------------------------------------------------------------------
double ofxGoIO::getMeasurementPeriod( int timeoutMs){
	auto ret = GoIO_Sensor_GetMeasurementPeriod( handle, timeoutMs);
	if( ofIsFloatEqual(ret, 1000000.0)){
		ofLogWarning("ofxGoIO::getMeasurementPeriod") << "failed";
	}else{
		measurementsInterval = ret;
	}
	return ret;
}
//---------------------------------------------------------------------------------------------
int ofxGoIO::getNumMeasurementsAvailable(){
	auto ret = GoIO_Sensor_GetNumMeasurementsAvailable(	handle );
	if(ret != 0){
		ofLogWarning("ofxGoIO::getNumMeasurementsAvailable") << "failed";
	}
	return ret;
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::calibrate( size_t numSamples){
	
	calibrationData.setup(numSamples);
	
	stateBeforeCalibration = getState();
	setState(OFX_GO_IO_STATE_CALIBRATING);

}
//---------------------------------------------------------------------------------------------
void ofxGoIO::updateCalibration(){
	if(getState() == OFX_GO_IO_STATE_CALIBRATING){
		size_t m = getNumMeasurementsAvailable();
		if(m > 0){
			if(calibrationData.currentSample < calibrationData.numSamples){
				auto n = GoIO_Sensor_ReadRawMeasurements(handle, &calibrationData.rawMeasurements[calibrationData.currentSample], std::min(m, calibrationData.numSamples - calibrationData.currentSample));
				calibrationData.currentSample += n;
			}else{
				getCurrentCalibrationFromDevice();
				calibrationData.process(handle);
				
				
				
				cout << "Calibration Ended. Average measurement: " << calibrationData.averageCalbMeasurement << currentDeviceCalibration.units << endl;
				
				setState(stateBeforeCalibration);
				
			}
		}
	}
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::updateMeasurements(){
	if(getState() == OFX_GO_IO_STATE_MEASURING){
		size_t m = getNumMeasurementsAvailable();
		
		lastMeasurement.data.resize(m);
		
		auto n = GoIO_Sensor_ReadRawMeasurements(handle,
												 lastMeasurement.data.data(),
												 m);
		lastMeasurement.aquisitionTime = ofGetElapsedTimef();
		lastMeasurement.sampleTimeInterval = measurementsInterval;
		if(n != m){
			ofLogError("ofxGoIO::updateMeasurements()") << "did not read all available measurements";
		}
		if(n){
			bHasNewData = true;
			unique_lock<std::mutex> lck(dataMutex);
			measurements[currentMeasurementIndex] = lastMeasurement;
			++currentMeasurementIndex %= measurements.size();
			
		}
		ofNotifyEvent(newMeasurementEvent, lastMeasurement, this);
		
	}
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::startMeasurements(int timeoutMs){
	if(getState() != OFX_GO_IO_STATE_MEASURING){
		if(setMeasurementPeriod(measurementsInterval, timeoutMs)){
			bool bRet = GoIO_Sensor_SendCmdAndGetResponse(handle, SKIP_CMD_ID_START_MEASUREMENTS, NULL, 0, NULL, NULL, timeoutMs) == 0;
			if(bRet){
				currentMeasurementIndex = 0;
				measurements.resize(OFX_GO_IO_DEFAULT_NUM_MEASUREMENTS);
				
				setState(OFX_GO_IO_STATE_MEASURING);
				return true;
			}
		}
	}
	ofLogError("ofxGoIO::startMeasurements") << "failed";
	return false;
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::stopMeasurements(int timeoutMs){
	if(getState() == OFX_GO_IO_STATE_MEASURING){
		if(GoIO_Sensor_SendCmdAndGetResponse(handle, SKIP_CMD_ID_STOP_MEASUREMENTS, NULL, 0, NULL, NULL, timeoutMs) == 0){
			setState(OFX_GO_IO_STATE_SETUP);
			return true;
		}
	}
	ofLogError("ofxGoIO::stopMeasurements") << "failed";
	return false;
}

//---------------------------------------------------------------------------------------------
bool ofxGoIO::shouldRepeatWithDelay(uint64_t& delay) {
	delay = measurementsInterval * 1000;
	return true;
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::threadedFunction(){
	if(getState() == OFX_GO_IO_STATE_CALIBRATING){
		updateCalibration();
	}
}
//---------------------------------------------------------------------------------------------
const ofxGoIODeviceCalibrationProfile& ofxGoIO::getCurrentCalibrationFromDevice(){

	if(GoIO_Sensor_DDSMem_GetActiveCalPage(handle, &currentDeviceCalibration.calPageIndex)== 0){
		char tmpstring[50];
		if(GoIO_Sensor_DDSMem_GetCalPage(handle,
										 currentDeviceCalibration.calPageIndex,
										 &currentDeviceCalibration.coeff[0],
										 &currentDeviceCalibration.coeff[1],
										 &currentDeviceCalibration.coeff[2],
										 tmpstring, sizeof(tmpstring)) == 0)
		{
			currentDeviceCalibration.units = tmpstring;
		}
		GoIO_Sensor_DDSMem_GetCalibrationEquation(handle, &currentDeviceCalibration.equationType);
	}
	return currentDeviceCalibration;
}
//---------------------------------------------------------------------------------------------
ofxGoIO::State ofxGoIO::getState(){
	return state;
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::setState(ofxGoIO::State newState){
	if(state != newState){
		state = newState;
		ofLogVerbose("ofxGoIO::setState") << " New state: \"" << stateToString(newState) << "\"";
		// do whatever needed for the new state
		
		if(!isRunning() && (state == OFX_GO_IO_STATE_MEASURING || state == OFX_GO_IO_STATE_CALIBRATING)){
			 start();
		}
		
	}else{
		ofLogVerbose("ofxGoIO::setState") << "State \"" << stateToString(newState) << "\" already set";
	}
}
void ofxGoIOMeasurement::addToMesh(ofMesh& mesh, const ofRectangle& viewport){
//	std::vector<int>data;
//	double aquisitionTime; // since app started running in seconds
//	double sampleTimeInterval; // time interval between samples in seconds
	
	float x=0;
	if(mesh.getNumVertices()){
		x = mesh.getVertices().back().x;
	}
	for(auto&d: data){
		
	}
	
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::draw(){
	
}
//---------------------------------------------------------------------------------------------
const ofxGoIODevice& ofxGoIO::getCurrentDevice(){
	return currentDevice;
}
