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
	void draw(const ofxGoIO& goIO, const ofRectangle& rect ){
		ofMesh mesh;
		mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
		size_t totalMeasurements = 0;
		for(size_t i = 0; i < goIO.measurements.size(); i++){
			totalMeasurements += goIO.measurements[i].data.size();
		}
		float xInc = rect.width / totalMeasurements;
		size_t vInd = 0;
		auto rmn = rect.getMinY();
		auto rmx = rect.getMaxY();
		for(size_t i = 0; i < goIO.measurements.size(); i++){
			auto& m = goIO.measurements[(i+goIO.currentMeasurementIndex)%goIO.measurements.size()];
			for(size_t j = 0; j < m.data.size(); j++){
				mesh.addVertex({vInd * xInc + rect.x,  ofMap(m.data[j], -32767, 32767, rmn, rmx) ,0.0f });
				++vInd;
			}
		}
		mesh.draw();
	}
};

