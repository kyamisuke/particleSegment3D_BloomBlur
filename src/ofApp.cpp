#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    //ofBackground(50);
    ofBackground(180, 255, 255);
    ofSetFrameRate(60);
    ofEnableAlphaBlending();
    ofSetLineWidth(5);
    ofSetVerticalSync(true);
    ofSetSphereResolution(10);
    
#ifdef TARGET_OPENGLES
    printf("TARGET_OPENGLES\n");
#else
    if(ofIsGLProgrammableRenderer()){
        printf("ofIsGL\n");
    }else{
        printf("else\n");
    }
#endif

    
    // load shader file
    if(!this->shader.load("shader.vert", "shader.frag")) { exit(); };
    
    blur.load("shader/blur.vert", "shader/blur.frag");
    bloom.load("shader/bloom.vert", "shader/bloom.frag");
    
    // allocate fbo
    this->fbo.allocate(ofGetWidth(), ofGetHeight());
    this->onePassFbo.allocate(ofGetWidth(), ofGetHeight());
    this->twoPassFbo.allocate(ofGetWidth(), ofGetHeight());
    
    // initialize y_noise
//    for (int i=0; i < ofGetHeight(); i++) { this->y_noise.push_back(0.f); }
    
    gui.setup();
    gui.setPosition(10, 10);
    gui.add(strength.set("strength", 6.0, 0.0, 20.0));

}

//--------------------------------------------------------------
void ofApp::update(){
    // fix to generate ranfom nomber
    ofSeedRandom(39);
    
    // begin fbo
    this->fbo.begin();
    this->cam.begin();
    
    // Setup cam
        float time = ofGetElapsedTimef();
        float r_ = 1500;
        cam.setPosition(r_ * ofMap(ofNoise(ofRandom(1000), ofGetFrameNum()*0.003), 0, 1, -1, 1), 1000 * sin(time), r_ *ofMap(ofNoise(ofRandom(1000), ofGetFrameNum()*0.003), 0, 1, -1, 1));
        cam.lookAt(ofVec3f(0, 0, 0), ofVec3f(0, -1, 0));

    
    ofClear(159, 255, 255);
    ofSetColor(0);
    ofNoFill();
    ofDrawSphere(ofVec3f(0), 850);
    ofFill();
    
    // generate location
    vector <ofVec3f> locations;
    
    for (int i=0; i < 70; i++) {
        auto location = glm::vec3(ofMap(ofNoise(ofRandom(1000), ofGetFrameNum()*0.003), 0, 1, -500, 500),
                                  ofMap(ofNoise(ofRandom(1000), ofGetFrameNum()*0.003), 0, 1, -500, 500),
                                  ofMap(ofNoise(ofRandom(1000), ofGetFrameNum()*0.003), 0, 1, -500, 500));
        locations.push_back(location);
    }
    
    
    auto r = 50.f;
    for (int i=0; i < locations.size(); i++) {
        auto min_dist = 300.f;
        for (int j=0; j < locations.size(); j++) {
            // compare any particles
            if (i == j) { continue; }
            
            // get distance and direction between i particle and j particle
            auto dist = ofDist(locations[i].x, locations[i].y, locations[i].z, locations[j].x, locations[j].y, locations[j].z);
            auto dir = ofVec3f(locations[j].x - locations[i].x, locations[j].y - locations[i].y, locations[j].z - locations[i].z);
            dir.normalize();
            // save now distance
            if (dist < min_dist) { min_dist = dist; };

            // calculate line-circle interseption
            auto point_i = ofVec3f(locations[i].x + dir.x * r, locations[i].y + dir.y * r, locations[i].z + dir.z * r);
            auto point_j = ofVec3f(locations[j].x - dir.x * r, locations[j].y - dir.y * r, locations[j].z - dir.z * r);
            
            // if distance is less than 200 then connect i and j particles
            if (dist < 300) {
                //ofSetColor(198, 156, 197, ofMap(dist, 0, 200, 255, 0));
                ofSetColor(0, ofMap(dist, 0, 300, 255, 0));
                ofNoFill();
                ofDrawSphere(point_i, r * 0.3);
                ofFill();
                ofDrawLine(point_i, point_j);
            }
        }
        
        // draw particle
        //ofSetColor(198, 156, 197);
        ofSetColor(0, 100);
        ofNoFill();
        ofDrawSphere(locations[i], r);
        
        ofSetColor(0);
        ofFill();
        ofDrawSphere(locations[i], ofMap(min_dist, 0, 300, r*0.7, 0));
    }
    
    // end fbo
    this->cam.end();
    this->fbo.end();
    
//    float noise_value;
//    for (int y=0; y < this->y_noise.size(); y++) {
//        if (y % 72 == 0) {
//            noise_value = ofMap(ofNoise(y * 0.05, ofGetFrameNum() * 0.03), 0, 1, -1, 1);
//
//            if      (noise_value > 0.65)    { noise_value -= 0.65; }
//            else if (noise_value < -0.65)   { noise_value += 0.65; }
//            else                            { noise_value = 0.0; }
//        }
//
//        this->y_noise[y] = noise_value;
//    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // X方向のガウシアンブラー
    onePassFbo.begin();
    
    blur.begin();
    
    blur.setUniformTexture("tex", fbo.getTexture(), 0);
    blur.setUniform1i("horizontal", true);
    blur.setUniform1f("strength", strength);
    fbo.draw(0, 0);
    
    blur.end();
    
    onePassFbo.end();
    //
    // Y方向のガウシアンブラー
    twoPassFbo.begin();
    
    blur.begin();
    blur.setUniformTexture("tex", onePassFbo.getTexture(), 0);
    blur.setUniform1i("horizontal", false);
    blur.setUniform1f("strength", strength);
    
    onePassFbo.draw(0, 0);
    
    blur.end();
    
    twoPassFbo.end();
    
    // ガウシアンブラーと元の描画内容を加算合成
    bloom.begin();
    bloom.setUniformTexture("tex", fbo.getTexture(), 0);
    bloom.setUniformTexture("blur", twoPassFbo.getTexture(), 1);
    
    fbo.draw(0, 0);
    
    bloom.end();
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
