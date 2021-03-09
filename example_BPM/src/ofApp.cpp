#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	measurementListener =  goIO.newMeasurementEvent.newListener( [&](ofxGoIOMeasurement& m){
		measurement = m;
        
//        ofLog()<<"data length "<<measurement.data().rawData; //<<" data.back() "<<measurement.data().back();
//        ofLog()<<"aquisitionTime "<<measurement.aquisitionTime;
//        ofLog()<<"sampleTimeInterval "<<measurement.sampleTimeInterval;
        
//        cout<<"measurement "<<measurement.size()<<endl;
//        for(auto&d: measurement){
//            cout  <<"     raw: " << d.rawData  << " Aquisition time:  " << d.aquisitionTime<<endl;
//        }
        
//        int abs_data = abs(measurement.back().rawData);
//        float timeDiff =  measurement.back().aquisitionTime - lastAquisitionTime;
//        cout<<"     timeDiff "<<timeDiff<<endl;
        
        float map_data = ofMap(measurement.back().rawData,-32767,32767,0,255,true);
        detected = heart_object.checkForBeat(map_data);
       
        if (detected == true)
        {
            cout<<"We sensed a beat!"<<endl;
            long delta = ofGetElapsedTimeMillis() - lastBeat;
            lastBeat = ofGetElapsedTimeMillis();
            
            beatsPerMinute = 60 / (delta / 1000.0);
            
            if (beatsPerMinute < 255 && beatsPerMinute > 20)
            {
                rates[rateSpot++] = (char)beatsPerMinute; //Store this reading in the array
                rateSpot %= RATE_SIZE; //Wrap variable
                
                //Take average of readings
                beatAvg = 0;
                for (char x = 0 ; x < RATE_SIZE ; x++){
                    beatAvg += rates[x];
                }
                beatAvg /= RATE_SIZE;
            }
        }

        lastAquisitionTime = measurement.back().aquisitionTime;
	});
    
	if(goIO.setup()){
		goIO.startMeasurements();
		maxPeriod = goIO.getMaximumMeasurementPeriod();
		minPeriod = goIO.getMinimumMeasurementPeriod();
		currentPeriod = goIO.getMeasurementPeriod() ;

	}
	rendererBuffer.resize(1000);
	for(size_t i = 0; i < rendererBuffer.size(); i++){
		rendererBuffer[i].rawData = ofMap(i, 0, rendererBuffer.size() - 1 ,-32767, 32767);
	}
	
	ofBitmapFont bf;
	cout << "ofBitmapFont rect: " << ofToString(bf.getBoundingBox(" ", 0, 0)) << endl;
	
	
	gui.setup();
	
	gui.add(x);
	gui.add(y);
	gui.add(margin);
	gui.add(padding);
	
	cam.removeAllInteractions();
	cam.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_XY, OF_MOUSE_BUTTON_LEFT);
	cam.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_Z, OF_MOUSE_BUTTON_RIGHT);
	cam.enableOrtho();
	cam.setNearClip(-1000000);
	cam.setFarClip(1000000);
	cam.setVFlip(true);
}

//--------------------------------------------------------------
void ofApp::update(){
	rendererBuffer[bufferIndex].rawData = ofGetMouseY();
	++bufferIndex %= rendererBuffer.size();
    
   
}
//--------------------------------------------------------------
void ofApp::exit(){
	goIO.close();
}
ofRectangle drawString(const string& str, const ofRectangle& refRect, bool bHorizontalStack, float margin, float padding, ofColor bgcolor, ofColor fgcolor = ofColor::black){
	ofBitmapFont bf;
	ofPushStyle();
	ofSetColor(bgcolor);
	glm::vec3 p;
	if(bHorizontalStack){
		p = refRect.getTopRight();
		p.x += margin;
		
	}else{
		p = refRect.getBottomLeft();
		p.y += margin;
	}
	p.x += padding;
	p.y += padding;
	p.y += 11;
	
	auto r = bf.getBoundingBox(str, p.x, p.y);
	r.x -= padding;
	r.y -= padding;
	r.height += padding*2;
	r.width += padding*2;
//	float h = r.height;
	ofDrawRectangle(r);
	
	ofSetColor(fgcolor);
	ofDrawBitmapString(str, p.x, p.y);
	
	ofPopStyle();
	
//	y += h + margin;
//	x = r.getMaxX() + margin;
	
	return r;
}
//--------------------------------------------------------------
void ofApp::draw(){
//	float y = 30;
//	float margin = 10;
//	float padding = 5;
//	float x = 30;
//
//	cam.begin();
	float mxy = 0;
	float mnx = x - padding;
	ofRectangle r(mnx, 0, 0,0);
	{
		stringstream ss;
		ss << goIO.getCurrentDevice();
		ss << endl;
		ss << "a"<< endl;
		ss << "Measurement period: " << endl;
		ss << "    current: " << currentPeriod << endl;
		ss << "    maximum: " << maxPeriod << endl;
		ss << "    minimum: " << minPeriod << endl;
		
		r = drawString(ss.str(), r, false, margin, padding, ofColor::yellow);
		mxy =  std::max(mxy, r.getMaxY());
	}
	{
		auto state = ofxGoIO::stateToString(goIO.getState());
		r = drawString("state:"+state, r, true, margin, padding, ofColor::magenta);
		mxy =  std::max(mxy, r.getMaxY());
	}
	{
		stringstream ss;
		ss << measurement;
		auto r2 = drawString(ss.str(), r, false, margin, padding, ofColor::cyan);
		mxy =  std::max(mxy, r2.getMaxY());
	}
	stringstream ss;
	ss << "App Framerate: " << ofGetFrameRate();
	r = drawString(ss.str(), r, true, margin, padding, ofColor::black, ofColor::white);
	mxy =  std::max(mxy, r.getMaxY());
//	if(bDraw){
	mxy += margin;
	waveRect.set( mnx, mxy, ofGetWidth() - 2*mnx,  ofGetHeight() - mxy - mnx );
	
//	renderer.draw(rendererBuffer, bufferIndex, waveRect, ofColor(40), ofColor(150));
    renderer.draw(goIO, waveRect, ofColor(40), ofColor(150));
    
    //MARK:draw BPM detection info
    ofPushMatrix();
    ofTranslate(ofGetWidth()-300, 10);
    int t_y = 0;
    ofSetColor(255);
    ofDrawBitmapStringHighlight("detected "+ofToString(detected), 0,t_y+=15);
    ofDrawBitmapStringHighlight("beatAvg "+ofToString(beatAvg), 0,t_y+=15);
    
    if(detected){
        ofFill();
        ofSetColor(255,0,0);
        ofDrawCircle(-20, 20, 10);
    }
    
    ofPopMatrix();
    
    //	}
    //x	xxx
    
//	cam.end();
	
//	gui.draw();
	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if(key == ' '){
		bDraw ^= true;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
