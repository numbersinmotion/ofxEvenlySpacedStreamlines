#include "ofxEvenlySpacedStreamlines.h"

ofxEvenlySpacedStreamlines::ofxEvenlySpacedStreamlines() {
    width = 100;
    height = 100;
    vectorField = [](glm::vec2 p)->glm::vec2{ return glm::vec2(1, 1); };
    separationField = [](glm::vec2 p)->float{ return 30; };
    dSeparationField = [](glm::vec2 p)->float{ return 0.5; };
    quadtree = ofxQuadtree<Particle>(ofRectangle(0, 0, width, height), 4);
    streamlines.clear();
}

ofxEvenlySpacedStreamlines::ofxEvenlySpacedStreamlines(int _width, int _height, function<glm::vec2(glm::vec2)> _vectorField, function<float(glm::vec2)> _separationField, function<float(glm::vec2)> _dSeparationField) {
    width = _width;
    height = _height;
    vectorField = _vectorField;
    separationField = _separationField;
    dSeparationField = _dSeparationField;
    quadtree = ofxQuadtree<Particle>(ofRectangle(0, 0, width, height), 4);
    streamlines.clear();
}

void ofxEvenlySpacedStreamlines::reset() {
    quadtree = ofxQuadtree<Particle>(ofRectangle(0, 0, width, height), 4);
    streamlines.clear();
}

void ofxEvenlySpacedStreamlines::calculateStreamlines(glm::vec2 seedPoint) {
    
    bool bBoundaryGood = isBoundaryGood(seedPoint);
    bool bSeparationGood = isSeparationGood(seedPoint);
    
    if ((bBoundaryGood && bSeparationGood) || streamlines.size() == 0) {
        
        ofPolyline tmpLine = getSingleStreamline(seedPoint);
        if (tmpLine.size() > 10) {
            tmpLine = tmpLine.getResampledBySpacing(5);
        }
        
        if (tmpLine.size() > 3) {
            
            for (int i = 0; i < tmpLine.size(); i++) {
                Particle p(tmpLine.getVertices()[i], streamlines.size());
                quadtree.put(make_shared<Particle>(p));
            }
            
            streamlines.push_back(tmpLine);
            
            for (int i = 0; i < tmpLine.size(); i++) {
                glm::vec2 p = tmpLine.getVertices()[i];
                glm::vec2 o = vectorField(p);
                o = glm::normalize(o);
                o = glm::rotate(o, float(0.5 * PI));
                o *= separationField(p);
                calculateStreamlines(p + o);
                calculateStreamlines(p - o);
            }
            
        }
    }
    
    return streamlines;
    
}

vector<shared_ptr<Particle>> ofxEvenlySpacedStreamlines::getNeighbors(glm::vec2 p, float size) {
    return quadtree.get(ofRectangle(p.x - size, p.y - size, 2 * size, 2 * size));
}

ofPolyline ofxEvenlySpacedStreamlines::getSingleStreamline(glm::vec2 seedPoint) {
    
    ofPolyline _tmpLine;
    _tmpLine.addVertex(seedPoint.x, seedPoint.y);
    
    bool done = false;
    bool flipped = false;
    
    while (!done) {
        glm::vec2 lastPoint = _tmpLine.getVertices()[_tmpLine.size() - 1];
        glm::vec2 direction = vectorField(lastPoint);
        
        if (flipped) direction *= -1;
        
        glm::vec2 newPoint = lastPoint + direction;
        
        _tmpLine.addVertex(newPoint.x, newPoint.y);
        
        bool bGood = true;
        if (bGood && !isBoundaryGood(newPoint)) bGood = false;
        if (bGood && !isSinkGood(glm::length(direction))) bGood = false;
        if (bGood && !isSeparationGood(newPoint)) bGood = false;
        if (bGood && !isSelfIntersectGood(_tmpLine, newPoint)) bGood = false;
        
        if (!bGood) {
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

void ofxEvenlySpacedStreamlines::drawStreamlines() {
    for (int i = 0; i < streamlines.size(); i++) {
        streamlines[i].draw();
    }
}

vector<vector<glm::vec2>> ofxEvenlySpacedStreamlines::getStreamlineMeshes() {
    
    vector<vector<glm::vec2>> out;
    
    for (int i = 0; i < streamlines.size(); i++) {
        
        ofPolyline line = streamlines[i].getResampledBySpacing(2);
        
        vector<glm::vec2> mesh;
        for (int j = 0; j < line.size(); j++) {
            
            glm::vec2 curr = line.getVertices()[j];
            
            glm::vec2 prev, next;
            if (j > 0) prev = line.getVertices()[j - 1];
            if (j < line.size() - 1) next = line.getVertices()[j + 1];
            
            float minDist = separationField(curr);
            vector<shared_ptr<Particle>> n = getNeighbors(curr, minDist);
            for (int k = 0; k < n.size(); k++) {
                float dist = ofDist(curr.x, curr.y, n[k]->getXPos(), n[k]->getYPos());
                if (dist < minDist && i != n[k]->lineIndex) minDist = dist;
            }
            
            float sep = separationField(curr);
            float dSep = dSeparationField(curr);
            float mag = (minDist - dSep * sep) / (sep - dSep * sep) + dSep;
            
            ofVec2f dir;
            if (j == 0) dir = next - curr;
            if (j == line.size() - 1) dir = curr - prev;
            else dir = next - prev;
            dir.normalize();
            dir.rotate(90);
            dir *= 1.0 * mag;
            
            glm::vec2 p1(curr.x + dir.x, curr.y + dir.y);
            glm::vec2 p2(curr.x - dir.x, curr.y - dir.y);
            
            mesh.push_back(p1);
            mesh.push_back(p2);
            
        }
        
        out.push_back(mesh);
    }
    
    return out;
    
}

void ofxEvenlySpacedStreamlines::drawStreamlineMeshes() {
    
    for (int i = 0; i < streamlines.size(); i++) {
        
        ofPolyline line = streamlines[i].getResampledBySpacing(2);
        
        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        for (int j = 0; j < line.size(); j++) {
            
            glm::vec2 curr = line.getVertices()[j];
            
            glm::vec2 prev, next;
            if (j > 0) prev = line.getVertices()[j - 1];
            if (j < line.size() - 1) next = line.getVertices()[j + 1];
            
            float minDist = separationField(curr);
            vector<shared_ptr<Particle>> n = getNeighbors(curr, minDist);
            for (int k = 0; k < n.size(); k++) {
                float dist = ofDist(curr.x, curr.y, n[k]->getXPos(), n[k]->getYPos());
                if (dist < minDist && i != n[k]->lineIndex) minDist = dist;
            }
            
            float sep = separationField(curr);
            float dSep = dSeparationField(curr);
            float mag = (minDist - dSep * sep) / (sep - dSep * sep) + dSep;
            
            ofVec2f dir;
            if (j == 0) dir = next - curr;
            if (j == line.size() - 1) dir = curr - prev;
            else dir = next - prev;
            dir.normalize();
            dir.rotate(90);
            dir *= 1.0 * mag;
            
            glm::vec3 p1(curr.x + dir.x, curr.y + dir.y, 0);
            glm::vec3 p2(curr.x - dir.x, curr.y - dir.y, 0);
            
            mesh.addVertex(p1);
            mesh.addColor(ofColor(0));
            mesh.addVertex(p2);
            mesh.addColor(ofColor(0));
            
        }
        
        mesh.draw();
        
    }
    
}

vector<ofPolyline> ofxEvenlySpacedStreamlines::getStreamlines() {
    return streamlines;
}

bool ofxEvenlySpacedStreamlines::isBoundaryGood(glm::vec2 p) {
    return p.x > 0 && p.x < width && p.y > 0 && p.y < height;
}

bool ofxEvenlySpacedStreamlines::isSeparationGood(glm::vec2 p) {
    vector<shared_ptr<Particle>> n = getNeighbors(p, separationField(p));
    float sep = separationField(p);
    float dSep = dSeparationField(p);
    for (int i = 0; i < n.size(); i++) {
        if (ofDist(p.x, p.y, n[i]->getXPos(), n[i]->getYPos()) < dSep * sep) {
            return false;
        }
    }
    return true;
}

bool ofxEvenlySpacedStreamlines::isSinkGood(float length) {
    return length > 0.01;
}

bool ofxEvenlySpacedStreamlines::isSelfIntersectGood(ofPolyline line, glm::vec2 newPoint) {
    if (line.size() > 0) {
        float sep = separationField(newPoint);
        float dSep = dSeparationField(newPoint);
        float length = ofDist(newPoint.x, newPoint.y, line.getVertices()[line.size() - 1].x, line.getVertices()[line.size() - 1].y);
        for (int i = line.size() - 1; i >= 0; i--) {
            glm::vec2 curr = line.getVertices()[i];
            if (length > dSep * sep) {
                if (ofDist(newPoint.x, newPoint.y, curr.x, curr.y) < dSep * sep) {
                    return false;
                }
            }
            if (i > 0) {
                glm::vec2 next = line.getVertices()[i - 1];
                length += ofDist(curr.x, curr.y, next.x, next.y);
            }
        }
    } else {
        return true;
    }
    return true;
}
