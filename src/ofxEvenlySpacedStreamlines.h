#pragma once

#include "ofMain.h"
#include "ofxQuadtree.h"

class Particle {
public:
    glm::vec2 position;
    int lineIndex;
    Particle(glm::vec2 _position, int _lineIndex) {
        position = _position;
        lineIndex = _lineIndex;
    }
    float getXPos() { return position.x; }
    float getYPos() { return position.y; }
};

class ofxEvenlySpacedStreamlines {
    
    public:
        
        ess();
        ess(int _width, int _height, function<glm::vec2(glm::vec2)> _vectorField, function<float(glm::vec2)> _separationField, function<float(glm::vec2)> _dSeparationField);
    
        void reset();
        void calculateStreamlines(glm::vec2 seedPoint);
        vector<ofPolyline> getStreamlines();
        void drawStreamlines();
        vector<vector<glm::vec2>> getStreamlineMeshes();
        void drawStreamlineMeshes();
    
    private:
    
        vector<shared_ptr<Particle>> getNeighbors(glm::vec2 p, float size);
        ofPolyline getSingleStreamline(glm::vec2 seedPoint);
        bool isBoundaryGood(glm::vec2 p);
        bool isSeparationGood(glm::vec2 p);
        bool isSinkGood(float length);
        bool isSelfIntersectGood(ofPolyline line, glm::vec2 newPoint);
    
        function<glm::vec2(glm::vec2)> vectorField;
        function<float(glm::vec2)> separationField;
        function<float(glm::vec2)> dSeparationField;
        int width, height;
        ofxQuadtree<Particle> quadtree;
        vector<ofPolyline> streamlines;
    
};
