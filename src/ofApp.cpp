#include "ofApp.h"


//For sort blobs by their x axis coordinates
bool my_compare(const ofxCvBlob & a, const ofxCvBlob & b)
{
    return (a.boundingRect.x < b.boundingRect.x);
}


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofBackground(224,234,221);
    //font settings
    ofTrueTypeFont::setGlobalDpi(72);
    AmaticB30.loadFont("Sansation_Light.ttf", 30, true, true);
    AmaticB30.setLineHeight(34.0f);
    AmaticB30.setLetterSpacing(1.035);
    notificationNum = 0;
    
    

    duration = 10;//play head scan rate
    volume = 10;
    threshold = 100;
    manualPlay_position =0;
    
    //camera view window size
    w = 600;
    h = 450;
   
    sp=4000;//the speed of play head    
    
    //drawn button area
    button_area_h = 0.1*h;
    button_area_w = 0.4*w;
    button_position_x = 0.6*w;
    button_position_y = 0.0*h;
    
    //calibration area
    calibration_area_h = 0.1*h;
    calibration_area_w = 0.1*w;
    calibration_position_x = 0.0*w;
    calibration_position_y = 0.0*h;
    
    //arrow play head area
    manualPlay_area_h = 0.1*h;
    manualPlay_area_w = w;
    manualPlay_position_x = 0.0*w;
    manualPlay_position_y = 0.9*h;
    
    //mode changes area
    switch_area_h = 0.15*h;
    switch_area_w = 0.1*w;
    switch_position_x = 0.9*w;
    switch_position_y = 0.15*h;


    //state flags
    setupNotification = true;
    playButtonFind = false;
    stopButtonFind = false;
    recordButtonFind = false;
    allButtonFind = false;
    IsDetectingFinger = false;
    s1 = false;
    s2 = false;
    s3 = false;
    uIsDraw = false;
    uIsPrepare = true;
    uIsRecord= false;
    uIsPlay=false;
    uIsStop=false;
    uIsLookforButton = false;
    /*
    bIsPressing=false;
    pIsPressing = false;
    nIsPressing = false;
    hIsPressing = false;
    backIsPressing = false;
    mpIsPressing = false;
     mpIsPressing = false;

     */
    flagforsp=true;
    calibration=false;
    optionSwitch=false;
    last_calibration = false;
       manualPlay = false;
    bLearnBakground = false;

    
   
    
    //----------------------------------------------
    
    movie.initGrabber(w, h, true);
    
    //reserve memory for cv images
    rgb.allocate(w, h);
    hsb.allocate(w, h);
    hue.allocate(w, h);
    sat.allocate(w, h);
    bri.allocate(w, h);
    filtered.allocate(w, h);
    filtered_2.allocate(w, h);
    fileterForSlide.allocate(w, h);
    
    
    //osc setup
    sender.setup(HOST, PORT);
    
  
    //settings for cv images
    grayBg_finger.allocate(button_area_w, button_area_h);
    grayDiff_finger.allocate(button_area_w, button_area_h);
    bri_finger.allocate(button_area_w, button_area_h);
    button_area_grey.allocate(button_area_w, button_area_h);
    calibration_area_grey.allocate(calibration_area_w, calibration_area_h);
    manualPlay_area_grey.allocate(manualPlay_area_w, manualPlay_area_h);
    switch_area_grey.allocate(switch_area_w, switch_area_h);



    
    //-------------------------------------------
    
    
    
    
    float marginX = ofGetWidth();
    float marginY = ofGetHeight();
    finishPrepare.set(marginX*0.4, marginY*0.8,marginX*0.15, marginX * 0.05);
    
    
}



void ofApp::update(){
    
    if (setupNotification){
        setupNotification = false;
        notificationNum = 1; //this is the index of voice prompts
     
        
    }
    
    ofxOscMessage m;
    m.setAddress("/notification");
    m.addIntArg(notificationNum);
    sender.sendMessage(m);
    
   
    
     movie.update();
    
    last_calibration = calibration;
    calibration = calibrationListener(); //check if calibration is finished
    if(calibration){
 
        uIsPrepare = true;
        updatePrepare();
        allButtonFind = false;
        stopButtonFind = false;
        playButtonFind = false;
        recordButtonFind = false;
        
        ofxOscMessage m;
        m.setAddress("/stop");
        m.addStringArg("bang");
        sender.sendMessage(m);
        

        
    }else if((!calibration)&&(last_calibration)){ // if calibration is not finished looking for buttons
        

        uIsLookforButton = true;
        uIsPrepare = false;
        ofSetColor(255, 0, 0);
    

        updateLookforButton();

    }else if(uIsStop)
    {
        updateStop();
        
    }else if(uIsRecord)
    {
        updateRecord();
        
    }else if(uIsPlay)
    {
        manualPlay = manualPlayListener(manualPlay_position);
        optionSwitch = switchListener();
        
        if(manualPlay){//if arrow card is found, set play head with arrow card
            playLine_x = manualPlay_position;
            ofSetColor(255, 0, 0);
        }else playLine_x = timeIndex * w * duration / 1000;
        //if there is no arrow card, play head scan from left to right
        
        updatePlay();
        
    }

}



bool ofApp::calibrationListener(){
    if (movie.isFrameNew()) {
        
        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels(), w, h);
        
        //mirror horizontal
        rgb.mirror(true, true);
        
        //duplicate rgb
        hsb = rgb;
        
        
        
        //convert to hsb
        hsb.convertRgbToHsv();
        
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);

        
    for (int i=0; i < calibration_area_w; i++)
    {
        
        for (int j=0; j < calibration_area_h; j++)
        {
            calibration_area_grey.getPixels()[(j* calibration_area_w + i)] =ofInRange(bri.getPixels()[((j+calibration_position_y)*w+(i+calibration_position_x))],0,threshold) ? 255 : 0;
            
            
        }
        
        
    }
        calibration_area_grey.flagImageChanged();
        
        //although it is called fingerFinder, here is a temp button finder
        calibrationFinder.findContours(calibration_area_grey,10,button_area_w*button_area_h/4,1,true,true);
    }

    if (calibrationFinder.blobs.size()==1) {
        return true;
    }else return false;

}




//this is for pressing drawn button
//looking for finger bolbs at button area, once find, change state
void ofApp::updateDetectFinger(){


     fingerFinder.findContours(grayDiff_finger,100,button_area_w*button_area_h/4,1,true,true);
    
    if(fingerFinder.nBlobs>0){
        
        finger_position_x = fingerFinder.blobs[0].boundingRect.x;
        finger_position_y = fingerFinder.blobs[0].boundingRect.y;
    
        if((playButton.inside(finger_position_x+button_position_x, finger_position_y+button_position_y)))
        {
            uIsPlay = true; uIsStop = false; uIsRecord = false; uIsPrepare = false;
            new_s = 1;
            old_s = new_s;
            
        }else if(recordButton.inside(finger_position_x+button_position_x, finger_position_y+button_position_y))
        {
            uIsRecord = true; uIsStop = false; uIsPlay = false; uIsPrepare = false;
            new_s = 2; 
            if (old_s != new_s)
            {
                ofxOscMessage m;
                m.setAddress("/record");
                m.addIntArg(1);
                sender.sendMessage(m);
                
                ofxOscMessage m2;
                m.setAddress("/stop");
                m.addIntArg(0);
                m.addIntArg(0);
                m.addIntArg(0);
                sender.sendMessage(m2);


            }
            old_s = new_s;
            
        }else if(stopButton.inside(finger_position_x+button_position_x, finger_position_y+button_position_y))
            
        {
            uIsStop = true; uIsPlay = false; uIsRecord = false; uIsPrepare = false;
            new_s = 3; 
            if (old_s != new_s)
            {
                ofxOscMessage m;
                m.setAddress("/stop");
                m.addIntArg(0);
                m.addIntArg(0);
                m.addIntArg(0);
                sender.sendMessage(m);

            }
            old_s = new_s;
            
            
            
        }
        
        
    }
}


//this is update funtion for calibration (drawing buttons and put sample color in the center)
void ofApp::updatePrepare(){
    
   
    
    
    if (calibrationListener()){
    
        notificationNum = 2;

    }
    
    ofxOscMessage m;
    m.setAddress("/notification");
    m.addIntArg(notificationNum);
    sender.sendMessage(m);
    
    
    
    
    if (movie.isFrameNew()) {
        
        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels(), w, h);
        
        //mirror horizontal
        rgb.mirror(true, true);
        
        //duplicate rgb
        hsb = rgb;
        
        
        
        //convert to hsb
        hsb.convertRgbToHsv();
        
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);
        
        for (int i=0; i < button_area_w; i++)
        {
            
            for (int j=0; j < button_area_h; j++)
            {
                button_area_grey.getPixels()[(j* button_area_w + i)] =ofInRange(bri.getPixels()[((j+button_position_y)*w+(i+button_position_x))],0,threshold) ? 255 : 0;
                
                
            }
            
            
        }
        
        button_area_grey.flagImageChanged();
        
        //although it is called fingerFinder, here is a temp button finder
        fingerFinder.findContours(button_area_grey,10,button_area_w*button_area_h/4,3,true,true);

        
        
        
    }//for movie grabber
    
    
}

// this is update funtion for recording
void ofApp::updateRecord(){
    
    ofxOscMessage m;
    m.setAddress("/stop");
    m.addStringArg("bang");
    sender.sendMessage(m);

    if (movie.isFrameNew()) {
        
        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels(), w, h);
        
        //mirror horizontal
        rgb.mirror(true, true);
        
        //duplicate rgb
        hsb = rgb;
        
        
        
        //convert to hsb
        hsb.convertRgbToHsv();
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);
        

        for (int i=0; i < button_area_w; i++)
        {
            
            for (int j=0; j < button_area_h; j++)
            {
                button_area_grey.getPixels()[(j* button_area_w + i)] =ofInRange(bri.getPixels()[((j+button_position_y)*w+(i+button_position_x))],0,threshold) ? 255 : 0;
                
                
            }
            
            
        }
        
        
        
        
        // take the abs value of the difference between background and incoming and then threshold:
        grayDiff_finger.absDiff(grayBg_finger, button_area_grey);
        grayDiff_finger.threshold(threshold);
        
        
        updateDetectFinger();



        //filter image based on the hue value were looking for
        for (int i=0; i<w*h; i++) {
            filtered.getPixels()[i] = ofInRange(hue.getPixels()[i],findHue-25,findHue+25) ? 255 : 0;
        }
        filtered.flagImageChanged();
        

        
        
    }//for movie grabber
    contours.findContours(filtered, 100, w*h/8, 5, false, false);
    std:ofSort(contours.blobs,my_compare);//sort bolbs by x position



}

//update funtion for detecting drawn buttons
void ofApp::updateLookforButton()
{
    
    if (movie.isFrameNew()) {
        
        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels(), w, h);
        
        //mirror horizontal
        rgb.mirror(true, true);
        
        //duplicate rgb
        hsb = rgb;
        
        
        
        //convert to hsb
        hsb.convertRgbToHsv();
        
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);
        
        for (int i=0; i < button_area_w; i++)
        {
            
            for (int j=0; j < button_area_h; j++)
            {
                button_area_grey.getPixels()[(j* button_area_w + i)] =ofInRange(bri.getPixels()[((j+button_position_y)*w+(i+button_position_x))],0,threshold) ? 255 : 0;
                
                
            }
            
            
        }

         button_area_grey.flagImageChanged();
        
        //although it is called fingerFinder, here is a temp button finder
        fingerFinder.findContours(button_area_grey,10,button_area_w*button_area_h/4,3,true,true);

        
    }//for movie grabber
    

    
    if (allButtonFind == false)
    {
    lookForButton();
    }
    
    if (allButtonFind) // once all button found, break from "looking for button state" and provide voice prompt
    {   notificationNum = 3;//"calibration finish"
        uIsLookforButton = false;
        uIsStop = true;
        
        ofxOscMessage m;
        m.setAddress("/notification");
        m.addIntArg(notificationNum);
        sender.sendMessage(m);



    }
    
    
}



void ofApp::updateStop(){
    
    ofxOscMessage m;
    m.setAddress("/stop");
    m.addStringArg("bang");
    //m.addIntArg(0);
    //m.addIntArg(0);
    sender.sendMessage(m);

    
    if (movie.isFrameNew()) {
        
        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels(), w, h);
        
        //mirror horizontal
        rgb.mirror(true, true);
        
        //duplicate rgb
        hsb = rgb;
        
        
        
        //convert to hsb
        hsb.convertRgbToHsv();
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);
        
        for (int i=0; i < button_area_w; i++)
        {
            
            for (int j=0; j < button_area_h; j++)
            {
                button_area_grey.getPixels()[(j* button_area_w + i)] =ofInRange(bri.getPixels()[((j+button_position_y)*w+(i+button_position_x))],0,threshold) ? 255 : 0;
                
                
            }
            
            
        }
        
        
        if (bLearnBakground == true){
            
            
            for (int i=0; i < button_area_w; i++)
            {
                
                for (int j=0; j < button_area_h; j++)
                {
                    grayBg_finger.getPixels()[(j* button_area_w + i)] =ofInRange(bri.getPixels()[((j+button_position_y)*w+(i+button_position_x))],0,threshold) ? 255 : 0;
                    
                    
                }
                
                
            }
            
            // the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBakground = false;
        }
        
        // take the abs value of the difference between background and incoming and then threshold:
        grayDiff_finger.absDiff(grayBg_finger, button_area_grey);
        grayDiff_finger.threshold(threshold);
        
        
        updateDetectFinger();
        
        
    }//for movie grabber

}



// send out intersection data to PD in play state
void ofApp::playSendData(){

if (playLine_x >= w) {
        timeIndex = 0;
        
    }
    
    if (timeIndex == 0)
    {
        beginTime = ofGetElapsedTimeMillis();
        timeIndex++;
        lastTime = beginTime;
   
    
    }
    
    
    
    currentTime = ofGetElapsedTimeMillis();
    
    
    
    if(((currentTime-lastTime)>duration)){
    lastTime = currentTime;
        
        
        
        
        for( int i = 0; i < contours.nBlobs; i++){
            
                ofxOscMessage m;
    m.setAddress("/intersection");
    m.addIntArg(i);//bolb id
    m.addFloatArg(timeIndex);//time index
        if((lowPoint[i])&&(highPoint[i])){
            
            if(optionSwitch)
            { m.addFloatArg(50.+(h-lowPoint[i])/27.);}//pitch
            else { m.addFloatArg(50+(h-lowPoint[i])/27);}

        }else{
            m.addIntArg(0);
        }
        m.addFloatArg(abs((lowPoint[i]-highPoint[i])*volume));//loudness

            
            
    sender.sendMessage(m);
        }
        
        for( int i = contours.nBlobs; i < maxBlobNum; i++){
            
            
            ofxOscMessage m;
            m.setAddress("/intersection");
            m.addIntArg(i);//bolb id
            m.addFloatArg(timeIndex);//time index
            
                m.addIntArg(0);
            
            m.addFloatArg(0);//loudness
            
            sender.sendMessage(m);
        }

        
        
        timeIndex++;
        ofSetColor(0,0,255);
        
        
    }
}


//update funciton for play state
void ofApp::updatePlay(){
    //run the contour finder on the filtered image to find blobs with a certain hue
   

    if (movie.isFrameNew()) {
        
        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels(), w, h);
        
        //mirror horizontal
        rgb.mirror(true, true);
        
        //duplicate rgb
        hsb = rgb;
        
        
        
        //convert to hsb
        hsb.convertRgbToHsv();
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);

        for (int i=0; i < button_area_w; i++)
        {
            
            for (int j=0; j < button_area_h; j++)
            {
                button_area_grey.getPixels()[(j* button_area_w + i)] =ofInRange(bri.getPixels()[((j+button_position_y)*w+(i+button_position_x))],0,threshold) ? 255 : 0;
                
                
            }
            
            
        }
        
        
        //filter image based on the hue value were looking for
        for (int i=0; i<w*h; i++) {
            filtered.getPixels()[i] = ofInRange(hue.getPixels()[i],findHue-25,findHue+25) ? 255 : 0;
        }
        filtered.flagImageChanged();

         // take the abs value of the difference between background and incoming and then threshold:
        grayDiff_finger.absDiff(grayBg_finger, button_area_grey);
        grayDiff_finger.threshold(threshold);
        
        
        updateDetectFinger();
        
        
        
    }
    
    
    contours.findContours(filtered, 100, w*h/8, maxBlobNum, false, false);
    std:ofSort(contours.blobs,my_compare);//sort bolbs by x position

    int num = contours.nBlobs;
    for (int k = 0; k < num; k++)
        
    {

    getIntersection(highPoint[k],lowPoint[k],contours.blobs[k]);
       

            }
    

    
    playSendData();
    
    
}


//this is a funtion for getting intersection from blob contour and play head
// Got help from code here https://github.com/bakercp/ofxFont/blob/master/src/ofFrameSetter.cpp#L204

void ofApp::getIntersection(int &y1, int &y2,const ofxCvBlob &blob ){
    
    
        ofPoint point1,point2;
    
        y1 = 0;
        y2 = 0;
        point1.set(playLine_x,0);
        point2.set(playLine_x,h);
        
        vector<ofPoint> intersections;
        
        intersections.push_back(point1); // add the first point to the beginning
        
        npts=blob.nPts;

    
        for(int i = 0; i < npts; i++) {
            int nextI = i != (npts - 1) ? i + 1 : 0;
            ofPoint intersection;
            if(ofLineSegmentIntersection(blob.pts[i], blob.pts[nextI], point1, point2, intersection)){
                float dist = intersection.distance(intersections[0]);
                bool didInsert = false;
                
                for(int j = 1; j < intersections.size(); j++) {
                    if(intersections[j].distance(intersections[0]) > dist) {
                        intersections.insert(intersections.begin() + j, intersection);
                        didInsert = true;
                        break;
                    }
                    
                }
                
                if(!didInsert) {
                    intersections.push_back(intersection); // if required, add it on the end
                }
                
            }
        }

    


    if((blob.boundingRect.x<=playLine_x)&&(playLine_x <=blob.boundingRect.x+blob.boundingRect.width)){
        
            y1 = intersections[1].y;
            y2 = intersections[intersections.size()-1].y;
           
        
    }else {
        y1= 0;
        y2 =0;
    }
    
    
    }





//--------------------------------------------------------------
void ofApp::draw(){
    
    
    
    ofSetColor(255,255,255);
    
    //draw all cv images
    rgb.draw(0,0);

    if(allButtonFind)
       {

        drawPaperButton();
       }



    
    if(uIsPrepare){
       
       drawPrepare();
        
        
    }
    
    else if(uIsLookforButton){
        
        
        drawLookforButton();
        
    } else if(uIsStop)
    {
        drawStop();

    } else if(uIsRecord){
        drawRecord();

    }else if(uIsPlay){
        drawPlay();

    }
    
    
    
}

void ofApp::drawPrepare(){

    button_area_grey.draw(w+button_position_x,0+button_position_y);
    fingerFinder.draw(w+button_position_x,0+button_position_y);

    

        ofFill();
    ofSetColor(0,0,0);
    AmaticB30.drawString("Calibrating", w*0.4, h*0.1);
        



}

void ofApp::drawLookforButton(){

    button_area_grey.draw(w+button_position_x,0+button_position_y);
    fingerFinder.draw(w+button_position_x,0+button_position_y);
    ofSetColor(0,0,0);
    AmaticB30.drawString("Looking for Buttons", w*0.45, h*0.1);





}



// this is for looking for drawn buttons. Once all button found, flag will be changed and button locations will be updated
void ofApp::lookForButton(){
    
    
    int centerxy;
    int leftdown;
    
    centerxy = 0.5*w*h+0.5*w;
    leftdown = 0.95*w*h+0.05*w;
    findHue = hue.getPixels()[centerxy];
    
    for (int i=0; i<fingerFinder.nBlobs; i++) {
    
        ofFill();
        
        
        ofSetColor(255,255,255);
        AmaticB30.drawString("drawButton:", w*0.5, h*0.2);


        int shape=contourShape(fingerFinder.blobs[i]);
        if ((shape == 1)&&(playButtonFind == false)){
            playButtonFind = true;
            playButton.set( fingerFinder.blobs[i].boundingRect.x+button_position_x, fingerFinder.blobs[i].boundingRect.y+button_position_y, fingerFinder.blobs[i].boundingRect.width, fingerFinder.blobs[i].boundingRect.height);
        } else if ((shape == 2)&&(recordButtonFind == false)){
            recordButtonFind = true;

            recordButton.set( fingerFinder.blobs[i].boundingRect.x+button_position_x, fingerFinder.blobs[i].boundingRect.y+button_position_y, fingerFinder.blobs[i].boundingRect.width, fingerFinder.blobs[i].boundingRect.height);
        } else if ((shape == 3)&&(stopButtonFind == false)){
            stopButtonFind = true;

            stopButton.set( fingerFinder.blobs[i].boundingRect.x+button_position_x, fingerFinder.blobs[i].boundingRect.y+button_position_y, fingerFinder.blobs[i].boundingRect.width, fingerFinder.blobs[i].boundingRect.height);
        }
    }
    
    if (stopButtonFind && recordButtonFind && playButtonFind )
    { allButtonFind = true; 
    uIsLookforButton = false;
    uIsStop = true;
    bLearnBakground = true;

    }
        
    
    
    
}


void ofApp::drawPaperButton(){
    
    
    ofSetColor(236,36,51);
    
    ofFill();
    if(uIsRecord)
    {ofNoFill();}
    ofRect(recordButton);
    ofFill();
    
    
    ofSetColor(251,153,32);
    if(uIsPlay)
    {ofNoFill();}
    ofRect(playButton);
    ofFill();
    
    ofSetColor(0,10,255);
    if(uIsStop)
    {ofNoFill();}

    ofRect(stopButton);
    ofFill();
    
    
    
    
 
    ofFill();
    
   
}


void ofApp::drawStop(){

    ofSetColor(0,10,255);
    

    button_area_grey.draw(w+button_position_x,0+button_position_y);
    fingerFinder.draw(w+button_position_x,0+button_position_y);
    AmaticB30.drawString("STOP", stopButton.x,stopButton.y+stopButton.height/4*10);
    

  
}

void ofApp::drawPlay(){
    ofSetColor(251,153,32);
    

    filtered.draw(w,0);
    contours.draw(w,0);
    calibrationFinder.draw(w,0);
    manualPlay_area_grey.draw(w,manualPlay_position_y);
    switch_area_grey.draw(w+ switch_position_x,switch_position_y);

    manualPlayFinder.draw(w,manualPlay_position_y);
    switchFinder.draw(w,switch_position_y);
    AmaticB30.drawString("PLAY", playButton.x+50,playButton.y+playButton.height/4*10);
    ofSetColor(0,0,0);
    if(optionSwitch)
    {AmaticB30.drawString("Continuous", switch_position_x-100,switch_position_y+0.15*h);}
    else { AmaticB30.drawString("Chromatic", switch_position_x-100,switch_position_y+0.15*h);}



    button_area_grey.draw(w+button_position_x,0+button_position_y);
    fingerFinder.draw(w+button_position_x,0+button_position_y);

    
    
  ofSetColor(255,0,0);
    ofLine(playLine_x, 0, playLine_x, h);   
}

void ofApp::drawRecord(){
    ofSetColor(236,36,51);
button_area_grey.draw(w+button_position_x,0+button_position_y);
    fingerFinder.draw(w+button_position_x,0+button_position_y);
    AmaticB30.drawString("RECORD", recordButton.x-50,recordButton.y+recordButton.height/4*10);

}


//this is for classifing buttons' shapes: triangle, round and rectangle
int ofApp::contourShape(ofxCvBlob blob)

{
    
    bool flag=true;
    bool flag2=true;
    bool flag3=true;
    
    
    int leftx=blob.boundingRect.x;
    int rightx=blob.boundingRect.x+blob.boundingRect.width;
    
    int point1x=leftx+ (rightx-leftx)/4;
    int point2x=leftx+ (rightx-leftx)/2;
    int point3x=leftx+ (rightx-leftx)/4*3;
    
    
    int point1y=blob.boundingRect.y+blob.boundingRect.height;
    int point2y=blob.boundingRect.y+blob.boundingRect.height;
    int point3y=blob.boundingRect.y+blob.boundingRect.height;
    float point1h=0;
    float point2h=0;
    float point3h=0;
    int maxh=0;
    
    
    for (int index=0; index <blob.nPts; index++)
    {
        
        if ((blob.pts[index].x >= (point1x-10))&&(blob.pts[index].x <= point1x)&&(blob.pts[index].y < point1y))
        {
            
            point1y=blob.pts[index].y;
            
        }
        
        if ((blob.pts[index].x >= (point2x-5))&&(blob.pts[index].x <= point2x+5)&&(blob.pts[index].y < point2y))
        {
            point2y=blob.pts[index].y;
            
        }
        if ((blob.pts[index].x >= (point3x))&&(blob.pts[index].x <= point3x+10)&&(blob.pts[index].y < point3y))
        {
            point3y=blob.pts[index].y;
        }
    }
    
    point1h= (blob.boundingRect.y+blob.boundingRect.height-point1y)/ blob.boundingRect.height;
    
    point2h= (blob.boundingRect.y+blob.boundingRect.height-point2y)/blob.boundingRect.height;
    
    point3h= (blob.boundingRect.y+blob.boundingRect.height-point3y)/blob.boundingRect.height;
    
    ofSetColor(0);
    
    if ((point1h> point2h)&&(point2h> point3h)&&(!playButtonFind))
        
    {return 1;
    }
    else if ((point1h+point3h)<(2*point2h)&&(!recordButtonFind))
    {
        return 2;}
    else
    {
        return 3;}
    
    
}


// check if there is arrow card in arrow play head area
bool ofApp::manualPlayListener(int &manualPlay_position){
    
    if (movie.isFrameNew()) {
        
        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels(), w, h);
        
        //mirror horizontal
        rgb.mirror(true, true);
        
        //duplicate rgb
        hsb = rgb;
        
        
        
        //convert to hsb
        hsb.convertRgbToHsv();
        
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);

        manualPlay_area_w = w;
        for (int i=0; i < manualPlay_area_w; i++)
        {
            for (int j=0; j < manualPlay_area_h; j++)
            {
                manualPlay_area_grey.getPixels()[(j* manualPlay_area_w + i)] =ofInRange(bri.getPixels()[((j+manualPlay_position_y)*w+(i+manualPlay_position_x))],0,threshold) ? 255 : 0;
                
            }
            
            
        }
        manualPlay_area_grey.flagImageChanged();
        
        //although it is called fingerFinder, here is a temp button finder
        manualPlayFinder.findContours(manualPlay_area_grey,10,manualPlay_area_w*manualPlay_area_h/8,1,true,true);
    }
    
    
    
    if(manualPlayFinder.nBlobs == 1){
    manualPlay_position = manualPlayFinder.blobs[0].boundingRect.x+manualPlayFinder.blobs[0].boundingRect.width/2;
    //cout << manualPlay_position << endl;
    }

    
    if (manualPlayFinder.nBlobs == 1) {
        return true;
        
    }else return false;

    
    
    
}


// check if there is mode card in mode card area
bool ofApp::switchListener(){
    
    if (movie.isFrameNew()) {
        
        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels(), w, h);
        
        //mirror horizontal
        rgb.mirror(true, true);
        
        //duplicate rgb
        hsb = rgb;
        
        
        
        //convert to hsb
        hsb.convertRgbToHsv();
        
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);
       
        manualPlay_area_w = w;
        for (int i=0; i < switch_area_w; i++)
        {
            //cout<<"i"<<i<<endl;
            for (int j=0; j < switch_area_h; j++)
            {
                switch_area_grey.getPixels()[(j* switch_area_w + i)] =ofInRange(bri.getPixels()[((j+switch_position_y)*w+(i+switch_position_x))],0,threshold) ? 255 : 0;
                
                
            }
            
            
        }
        switch_area_grey.flagImageChanged();
        
        //although it is called fingerFinder, here is a temp button finder
        switchFinder.findContours(switch_area_grey,50,switch_area_w*switch_area_h,1,true,true);
    }
    
    
    
  
    
    if (switchFinder.nBlobs == 1) {
        return true;
        
    }else return false;
    
    
    
    
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
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
