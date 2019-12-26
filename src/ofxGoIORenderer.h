//
//  ofxGoIORenderer.hpp
//  example
//
//  Created by Roy Macdonald on 12/25/19.
//

#pragma once
#include "ofxGoIO.h"
#include "ofMain.h"

class  ofxGoIORenderer{
public:
	
//	ofxGoIORenderer(){
		
//		signalMesh.setUsage(GL_DYNAMIC_DRAW);
		
//	}
	
	void draw(const ofxGoIOMeasurement& measurements, size_t currentIndex, const ofRectangle& rect, const ofColor& bgColor, const ofColor& signalColor  ){
		ofMesh signalMesh;
		signalMesh.setMode(OF_PRIMITIVE_LINE_STRIP);
//		auto& verts = signalMesh.getVertices();
//		if(verts.size() != measurements.size()){
//			verts.resize(measurements.size());
//		}
		
//		measurements.front().rawData = -32767;
//		measurements.back().rawData = 32767;
		
		float xInc = rect.width / measurements.size();
//		size_t vInd = 0;
		
		auto rmn = rect.getMinY();
		auto rmx = rect.getMaxY();
		
		size_t ind = 0;
		for(size_t i = 0; i < measurements.size(); i++){
			ind = (i + currentIndex) % measurements.size();
			auto& m = measurements[ind];
//			verts[i].x = (measurements.size() - i - 1) * xInc + rect.x;
//			verts[i].y = ofMap(m.rawData, -32767, 32767, rmn, rmx);
//			verts[i].z = 0.0f;
			
			signalMesh.addVertex({(measurements.size() - i - 1) * xInc + rect.x, ofMap(m.rawData, -32767, 32767, rmn, rmx), 0.0f});
		}
	
		ofSetColor(bgColor);
		ofDrawRectangle(rect);
		ofSetColor(signalColor);
		
		signalMesh.draw();
		ofPushStyle();
		ofSetColor(ofColor::red);
		auto x =  currentIndex * xInc + rect.x ;
		ofDrawLine(x, rmn, x, rmx);
		ofPopStyle();
	}
	void draw(const ofxGoIO& goIO , const ofRectangle& rect, const ofColor& bgColor, const ofColor& signalColor  ){
		draw(goIO.measurementsBuffer, goIO.currentMeasurementIndex, rect, bgColor, signalColor);
	}
};

