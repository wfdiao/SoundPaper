#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"


// for osc
#define HOST "localhost"
#define PORT 12345

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    
    void draw();
    void drawRecord();
    void drawPlay();
    void drawStop();
    void drawPaperButton();
    void drawPrepare();
    void drawLookforButton();

    void updateRecord();
    void updatePlay();
    void updatePrepare();
    void updateStop();
    void updateLookforButton();
    
    
    bool calibrationListener();//check if calibration card are put on table
    void getIntersection(int &y1, int &y2,const ofxCvBlob &blob );// get intersections of blob's contour and play head
    bool manualPlayListener(int &manualPlay_position); // check if there is arrow card in arrow play head area
    bool switchListener();// check if there is mode card in mode card area


    void playSendData();

    
    int beginTime,currentTime,lastTime;
    int timeIndex;
    int playLine_x;
    int duration;
    int npts;
    int volume;
    int highPoint[8]={0,0,0,0,0,0,0,0};
    int lowPoint[8]={0,0,0,0,0,0,0,0};
    int y1,y2;//intersections' y
    int manualPlay_area_w,manualPlay_area_h;
    int manualPlay_position_x,manualPlay_position_y;
    int switch_area_w,switch_area_h;
    int switch_position_x,switch_position_y;
    int threshold;
    int manualPlay_position;

    
    void lookForButton();// look for drawn buttons
    
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    int contourShape(ofxCvBlob bolb);//classify button's shape. return shape index
    void updateDetectFinger();// detect finger pressing drawn buttons
    void drawButton();
    
    
    
    
    //buttons
    ofRectangle finishPrepare;//finish all prepare work
    ofRectangle recordButton;
    ofRectangle stopButton;
    ofRectangle playButton;
    
    bool uIsPrepare;
    bool uIsDraw;
    bool uIsRecord;
    bool uIsPlay;
    bool uIsMultiPlay;
    bool uIsStop;
    bool uIsLookforButton;
    
    bool flagforsp;
    bool IsDetectingFinger;
    bool s1,s2,s3;
    int old_s = 0;
    int new_s = 0;
    bool new_status;
    bool calibration,last_calibration;
    bool manualPlay, last_manualPlay;
    bool optionSwitch;

    /*
     bool bIsPressing;//record
     bool pIsPressing;//play
     bool nIsPressing;//next
     bool backIsPressing;//back
     bool hIsPressing;
     bool mpIsPressing;//multiplay
     */
    
    
    //Opencv
    ofVideoGrabber movie;
    
    ofxCvColorImage rgb,hsb;
    ofxCvGrayscaleImage hue,sat,bri,filtered, filtered_2,fileterForSlide;
    ofxCvContourFinder contours,contoursForSlide;
    ofxCvGrayscaleImage     grayBg_finger;
    ofxCvGrayscaleImage     grayDiff_finger;
    ofxCvGrayscaleImage     bri_finger;
    ofxCvGrayscaleImage     fingerImage;
    ofxCvGrayscaleImage     calibration_area_grey;
    ofxCvContourFinder contours_2;
    ofxCvContourFinder fingerFinder;
    ofxCvContourFinder calibrationFinder;
    ofxCvContourFinder manualPlayFinder;
    ofxCvGrayscaleImage     manualPlay_area_grey;
    ofxCvContourFinder switchFinder;
    ofxCvGrayscaleImage     switch_area_grey;
    
    
    ofxBlobTracker _blobTracker;
    ofxBlobTracker fingerTracker;
    ofxBlobTracker calibrationTracker;

    
    
    int w,h;
    int findHue,findHue2;
    float sp;//play head speed
    float slideH;
    int playTime;
    int line_position_x;
    
    int button_area_w,button_area_h;
    int button_position_x,button_position_y;
    int calibration_area_w,calibration_area_h;
    int calibration_position_x,calibration_position_y;
    ofxCvColorImage button_area;
    ofxCvGrayscaleImage button_area_grey;
    
    int finger_position_x,finger_position_y;
    
    
    
    //Osc
    ofxOscSender sender;
    ofBuffer imgAsBuffer;
    ofImage img;
    // Image to be displayed
    ofImage myImage;
    string log ;
    bool bLearnBakground;
    
    bool playButtonFind;
    bool stopButtonFind;
    bool recordButtonFind;
    bool allButtonFind;
    bool setupNotification;
    
    int maxBlobNum = 8;
    int notificationNum;
    
    
    ofTrueTypeFont AmaticB30;
};
