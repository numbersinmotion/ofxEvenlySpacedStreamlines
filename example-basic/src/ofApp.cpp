#include "ofApp.h"

ofVec2f getVector(ofVec2f p, float t) {
    
    float a = ofMap(ofNoise(p.x / 300, p.y / 300, t), 0, 1, 0, 2 * PI);
    float r = ofMap(ofNoise(p.x / 300, p.y / 300, t + 1000), 0, 1, 0, 10);
    return ofVec2f(r * cos(a), r * sin(a));
    
}

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofBackground(255);
    
    ess.setup(10, 0.5, ofGetWidth(), ofGetHeight());
    
    ess.getStreamlines(ofVec2f(ofGetWidth() / 2, ofGetHeight() / 2), getVector);

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ess.draw();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    ess.setup(10, 0.5, ofGetWidth(), ofGetHeight());
    
    ess.getStreamlines(ofVec2f(ofGetWidth() / 2, ofGetHeight() / 2), getVector);

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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
