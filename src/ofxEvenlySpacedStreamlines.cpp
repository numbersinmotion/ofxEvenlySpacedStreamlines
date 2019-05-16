
#include "ofxEvenlySpacedStreamlines.h"

void ofxEvenlySpacedStreamlines::setup(float _sep, float _dSep, int _width, int _height) {
    
    sep = _sep;
    
    dSep = _dSep;
    
    time = 0.2 * ofGetElapsedTimef();
    
    width = _width;
    height = _height;
    
    xBins = ceil(width / sep);
    yBins = ceil(height / sep);
    
    bins.clear();
    bins.resize(xBins * yBins);
    
    streamlines.clear();
    
}

vector<ofPolyline> ofxEvenlySpacedStreamlines::getStreamlines(ofVec2f seedPoint, ofVec2f (*getVector)(ofVec2f, float)) {
    
    // is valid seed point?
    bool badBoundary = seedPoint.x < 0 || seedPoint.x > width || seedPoint.y < 0 || seedPoint.y > height;
    bool badSeparation = false;
    vector<particle> n = getNeighbors(seedPoint.x, seedPoint.y);
    for (int i = 0; i < n.size(); i++) {
        if (ofDist(seedPoint.x, seedPoint.y, n[i].pos.x, n[i].pos.y) < dSep * sep) {
            badSeparation = true;
            break;
        }
    }
    
    if ((!badBoundary && !badSeparation) || streamlines.size() == 0) {
        
        ofPolyline tmpLine = getSingleStreamline(seedPoint, getVector);
//        if (tmpLine.size() > 20) {
//            tmpLine = tmpLine.getResampledBySpacing(5);
//        }
        
        if (tmpLine.size() > 10) {
            
            for (int i = 0; i < tmpLine.size(); i++) {
                particle p(tmpLine.getVertices()[i], streamlines.size());
                int xBin = p.pos.x / sep;
                int yBin = p.pos.y / sep;
                int bin = yBin * xBins + xBin;
                if(xBin >= 0 && xBin < xBins && yBin >= 0 && yBin < yBins) bins[bin].push_back(p);
            }
            
            streamlines.push_back(tmpLine);
            
            for (int i = 0; i < tmpLine.size(); i++) {
                ofVec2f p = tmpLine.getVertices()[i];
                ofVec2f o = getVector(p, time);
                o.normalize();
                o.rotate(90);
                o *= sep;
//                if (streamlines.size() < 10) {
                    getStreamlines(p + o, getVector);
                    getStreamlines(p - o, getVector);
//                }
            }
            
        }
    }
    
    return streamlines;
    
}

vector<particle> ofxEvenlySpacedStreamlines::getNeighbors(float x, float y) {
    
    vector<particle> neighbors;
    
    int xBin = x / sep;
    int yBin = y / sep;
    int bin = yBin * xBins + xBin;
    
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (xBin + i >= 0 && xBin + i < xBins && yBin + j >= 0 && yBin + j < yBins) {
                int binIndex = (yBin + j) * xBins + (xBin + i);
                vector<particle> particles = bins[binIndex];
                neighbors.insert(neighbors.end(), particles.begin(), particles.end());
            }
        }
    }
    
    return neighbors;
    
}

ofPolyline ofxEvenlySpacedStreamlines::getSingleStreamline(ofVec2f seedPoint, ofVec2f (*getVector)(ofVec2f, float)) {
    
    ofPolyline _tmpLine;
    _tmpLine.addVertex(seedPoint.x, seedPoint.y);
    
    bool done = false;
    bool flipped = false;
    
    while (!done) {
        
        ofVec2f lastPoint = _tmpLine.getVertices()[_tmpLine.size() - 1];
        ofVec2f direction = getVector(lastPoint, time);
        
        if (flipped) direction *= -1;
        
        ofVec2f newPoint = lastPoint + direction;
        
        _tmpLine.addVertex(newPoint.x, newPoint.y);
        
        bool checkBoundary = newPoint.x < 0 || newPoint.x > width || newPoint.y < 0 || newPoint.y > height;
        bool checkSink = direction.length() < 0.1;
        bool checkSeparation = false;
        
        vector<particle> n = getNeighbors(newPoint.x, newPoint.y);
        for (int i = 0; i < n.size(); i++) {
            if (ofDist(newPoint.x, newPoint.y, n[i].pos.x, n[i].pos.y) < dSep * sep) {
                checkSeparation = true;
                break;
            }
        }
        
        bool checkSelfIntersect = false;
        if (!checkBoundary && !checkSink && !checkSeparation) {
//            float length = tmpLine.getPerimeter();
            
            float length = 0;
            if (_tmpLine.size() > 1) {
                for (int i = 0; i < _tmpLine.size() - 1; i++) {
                    glm::vec3 curr = _tmpLine.getVertices()[i];
                    glm::vec3 next = _tmpLine.getVertices()[i + 1];
                    length += ofDist(curr.x, curr.y, next.x, next.y);
                }
            }
            
            float distToLast = ofDist(newPoint.x, newPoint.y, lastPoint.x, lastPoint.y);
            for (int i = 0; i < _tmpLine.size() - 1; i++) {
                if (length > dSep * sep) {
                    if (ofDist(newPoint.x, newPoint.y, _tmpLine.getVertices()[i].x, _tmpLine.getVertices()[i].y) < dSep * sep && length > dSep * sep) {
                        checkSelfIntersect = true;
                        break;
                    }
                    ofVec2f checkCurr = _tmpLine.getVertices()[i];
                    ofVec2f checkNext = _tmpLine.getVertices()[i + 1];
                    length -= ofDist(checkCurr.x, checkCurr.y, checkNext.x, checkNext.y);
                } else {
                    break;
                }
            }
        }
        
        if (checkBoundary || checkSink || checkSeparation || checkSelfIntersect || _tmpLine.size() > 50) {
            if (!flipped) {
                reverse(_tmpLine.begin(), _tmpLine.end());
                flipped = true;
            } else {
                done = true;
            }
        }
        
    }
    
    return _tmpLine;
    
}

void ofxEvenlySpacedStreamlines::draw() {
    
    for (int i = 0; i < streamlines.size(); i++) {
        
        ofPolyline line = streamlines[i];
        
        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        
        for (int j = 0; j < line.size(); j++) {
            
            ofVec2f curr = line.getVertices()[j];
            
            ofVec2f prev, next;
            if (j > 0) prev = line.getVertices()[j - 1];
            if (j < line.size() - 1) next = line.getVertices()[j + 1];
            
            float minDist = sep;
            vector<particle> n = getNeighbors(curr.x, curr.y);
            for (int k = 0; k < n.size(); k++) {
                float dist = ofDist(curr.x, curr.y, n[k].pos.x, n[k].pos.y);
                if (dist < minDist && i != n[k].index) minDist = dist;
            }
            
            float mag = (minDist - dSep * sep) / (sep - dSep * sep) + dSep;
            
            ofVec2f dir;
            if (j == 0) dir = next - curr;
            if (j == line.size() - 1) dir = curr - prev;
            else dir = next - prev;
            dir.normalize();
            dir.rotate(90);
            dir *= mag;
            
            glm::vec3 p1(curr.x + dir.x, curr.y + dir.y, 0);
            glm::vec3 p2(curr.x - dir.x, curr.y - dir.y, 0);
            
            mesh.addVertex(p1);
            mesh.addVertex(p2);
            mesh.addColor(ofColor(0));
            mesh.addColor(ofColor(0));
            
        }
        
        mesh.draw();
        
    }
    
}

vector<vector<glm::vec3>> ofxEvenlySpacedStreamlines::getMeshes() {
    
    vector<vector<glm::vec3>> out;
    
    for (int i = 0; i < streamlines.size(); i++) {
        
        ofPolyline line = streamlines[i].getResampledBySpacing(2);
        
        vector<glm::vec3> mesh;
        for (int j = 0; j < line.size(); j++) {
            
            ofVec2f curr = line.getVertices()[j];
            
            ofVec2f prev, next;
            if (j > 0) prev = line.getVertices()[j - 1];
            if (j < line.size() - 1) next = line.getVertices()[j + 1];
            
            float minDist = sep;
            vector<particle> n = getNeighbors(curr.x, curr.y);
            for (int k = 0; k < n.size(); k++) {
                float dist = ofDist(curr.x, curr.y, n[k].pos.x, n[k].pos.y);
                if (dist < minDist && i != n[k].index) minDist = dist;
            }
            
            float mag = (minDist - dSep * sep) / (sep - dSep * sep) + dSep;
            
            ofVec2f dir;
            if (j == 0) dir = next - curr;
            if (j == line.size() - 1) dir = curr - prev;
            else dir = next - prev;
            dir.normalize();
            dir.rotate(90);
            dir *= mag;
            
            glm::vec3 p1(curr.x + dir.x, curr.y + dir.y, 0);
            glm::vec3 p2(curr.x - dir.x, curr.y - dir.y, 0);
            
            mesh.push_back(p1);
            mesh.push_back(p2);
            
        }
        
        out.push_back(mesh);
    }
    
    return out;
    
}
