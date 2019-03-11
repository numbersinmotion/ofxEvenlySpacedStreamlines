#pragma once

#include "ofMain.h"

class particle {
    public:
        ofVec2f pos;
        int index;
        particle(ofVec2f pos, int index) : pos(pos), index(index) {}
};

class ofxEvenlySpacedStreamlines {
    
    public:
    
        void setup(float sep, float dSep, int width, int height);
        vector<ofPolyline> getStreamlines(ofVec2f seedPoint, ofVec2f (*getVector)(ofVec2f, float));
        void draw();
    
    private:
    
        ofPolyline getSingleStreamline(ofVec2f seedPoint, ofVec2f (*getVector)(ofVec2f, float));
        vector<particle> getNeighbors(float x, float y);
    
        float sep, dSep;
        float time;
        int width, height;
        int xBins, yBins;
        vector<vector<particle>> bins;
        vector<ofPolyline> streamlines;
    
};
