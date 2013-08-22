#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofEnableSmoothing();
	
	// ****   Video **** //
	id = 0; // Device id
	w_cam = 640/2;
	h_cam = 480/2;
	
	
	elVideo.listDevices();
	elVideo.setDeviceID(id);
	elVideo.initGrabber( w_cam , h_cam );
	
	// **** Image allocation **** //
	
	colorImg.allocate( w_cam, h_cam );
	escalaDeGrises.allocate( w_cam, h_cam );
	imgRestada.allocate( w_cam, h_cam );
	bkg.allocate( w_cam, h_cam );
	bkgGrises.allocate( w_cam, h_cam );
	thresholded.allocate( w_cam, h_cam );
	labeledBlobsImage.allocate( w_cam, h_cam );
	
	/*
	 
	 IplImage *theImage = thresholded.getCvImage();
	 IplImage *labelImage = cvCreateImage( cvGetSize(theImage) , IPL_DEPTH_LABEL, 1);
	 IplImage *imprueba = cvCreateImage( cvGetSize(theImage) , 8, 3);
	 
	 
	 
	 */
	/*thresholdedIpl = cvCreateImage(cvSize(w_cam, h_cam), 8, 1);
	 labelImage = cvCreateImage( cvGetSize(thresholdedIpl) , IPL_DEPTH_LABEL, 1);
	 imprueba = cvCreateImage( cvGetSize(thresholdedIpl) , 8, 3);
	 */
	
	// **** Allocation of UI images (for display) ***** //
	ofUntouchedSrc.setFromPixels( colorImg.getPixelsRef() );
	ofBkg.setFromPixels( bkg.getPixelsRef() );
	ofSrc.setFromPixels( colorImg.getPixelsRef() );
	ofSub.setFromPixels( imgRestada.getPixelsRef() );
	ofThres.setFromPixels( thresholded.getPixelsRef() );
	ofBlobs.setFromPixels( colorImg.getPixelsRef() );
	
	// **** Detection Parameters *****//
	
	bkgBlurFact = 5;
	sourceBlurFact = 5;
	remapLowValue = 0;
	remapHighValue = 255;
	contrastStretch = false;
	minBlobArea = 0;
	maxBlobArea = w_cam*h_cam*0.5;
	threshold = 80;
	
	// ****** OSC ******* //
	
	HOST = "127.0.0.1";
	PORT = 9001;
	oscSender.setup( HOST, PORT );
	sendThroughOsc = true;
	
	setupGUI();
	
	// Pruebas:
	
	laPruebaExterna.loadImage("sliderhandle.png");
	
}

void testApp::update(){
	
	elVideo.update();
	
	if( elVideo.isFrameNew() ){
		colorImg.setFromPixels( elVideo.getPixels(), w_cam, h_cam);
		ofUntouchedSrc.setFromPixels( colorImg.getPixelsRef() );
		
		// ************ PROCEDURE FOR BLOB DETECTION ******************
		
		// Select channel (selected by vRadio)
		if(selectedChannel == grayscale){
			escalaDeGrises = colorImg; // Implicit conversion to grayscale
			bkgGrises = bkg;
			
		}
		else{
			colorImg.convertToGrayscalePlanarImage(escalaDeGrises, selectedChannel ) ;
			bkg.convertToGrayscalePlanarImage( bkgGrises, selectedChannel);
		}
		
		
		// Blur both of them
		
		bkgGrises.blur( bkgBlurFact ); // taking value from sliders!
		escalaDeGrises.blur( sourceBlurFact ); 
		
		// Set to ofImages for display
		/**/ ofBkg.setFromPixels( bkgGrises.getPixelsRef());
		/**/ ofSrc.setFromPixels( escalaDeGrises.getPixelsRef());
		
		// 
		imgRestada.absDiff( escalaDeGrises,  bkgGrises );
		if( contrastStretch == true){
			imgRestada.contrastStretch();
		}
		else{
			imgRestada.convertToRange(remapLowValue, remapHighValue);
		}
		
		
		// ** Subtracted to display image
		ofSub.setFromPixels( imgRestada.getPixelsRef());
		
		thresholded = imgRestada;
		thresholded.threshold( threshold );
		
		// ** Threshold to display image
		ofThres.setFromPixels( thresholded.getPixelsRef() );
		
		//IplImage *thresholdedIpl = thresholded.getCvImage();
		thresholdedIpl = thresholded.getCvImage();
		
		//IplImage *labelImage = cvCreateImage( cvGetSize(thresholdedIpl) , IPL_DEPTH_LABEL, 1);
	    IplImage *labelImage;
		labelImage = cvCreateImage( cvGetSize(thresholdedIpl) , IPL_DEPTH_LABEL, 1);
		
		
		//IplImage *iplBlobsImage = cvCreateImage( cvGetSize(thresholdedIpl) , 8, 3);
		IplImage *iplBlobsImage;
		iplBlobsImage = cvCreateImage( cvGetSize(thresholdedIpl) , 8, 3);
		
		
		
		unsigned int result = cvLabel(thresholdedIpl , labelImage , blobs);
		
		//		cout << "BLOB NUM " << blobs.size() <<endl;
		
		
		cvFilterByArea(blobs, minBlobArea , maxBlobArea);
		
		
		cvZero(iplBlobsImage); // Blank iplBlobsImage
		cvRenderBlobs(labelImage, blobs, thresholdedIpl, iplBlobsImage,  CV_BLOB_RENDER_CENTROID|
					  CV_BLOB_RENDER_BOUNDING_BOX|
					  CV_BLOB_RENDER_ANGLE|
					  CV_BLOB_RENDER_COLOR);		
		
		
		labeledBlobsImage.allocate( w_cam, h_cam );		
		labeledBlobsImage = iplBlobsImage;
		
		// ** Blobs to display image
		ofBlobs.setFromPixels( labeledBlobsImage.getPixelsRef() );
		
		// RELEASE
		cvReleaseImage(&labelImage);
		
		cvReleaseImage(&iplBlobsImage);
		// ******** SEND BLOBS THROUGH OSC ******* //
		
		if ( blobs.size() > 0  && sendThroughOsc == true ) {
			
			for(CvBlobs::const_iterator it = blobs.begin() ; it !=blobs.end(); it++){
				//cout << "iterating over blobs. Size of vec: " << blobs.size() << "| i: " << i << endl;
				CvBlob * blob = it->second;
				ofxOscMessage m;
				m.setAddress("/blob");
				
				// Send data in normalized coordinates
				m.addFloatArg(blob->centroid.x / w_cam);
				m.addFloatArg(blob->centroid.y / h_cam);
				m.addFloatArg( cvAngle( blob ));
				oscSender.sendMessage( m );
			}
		}
		
		
		
	}
	
	
	
	
}



void testApp::guiEvent( ofxUIEventArgs &e){
	//--- GUI Events for the left pane ----
	
	string name = e.widget->getName(); 
	int kind = e.widget->getKind(); 
	// ******* BLUR ******* //
	if (name == "BLUR") {
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		bkgBlurFact = slider->getScaledValue();
		slider->setValue( bkgBlurFact );
	}
	else if ( name == "BLUR_S" ){
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		sourceBlurFact = slider->getScaledValue();
		slider->setValue( sourceBlurFact );
	}
	// ******* Channel Radio ******* //
	else if ( name == "Red"){
		//Select red as source for blob detection
		selectedChannel  = red;
		cout << "red" << endl;
	}
	else if ( name == "Green"){
		selectedChannel = green;
		cout << "green" << endl;
	}
	else if ( name == "Blue"){
		selectedChannel = blue;
		cout << "blue" << endl;
	}
	else if ( name == "Grayscale"){
		selectedChannel = grayscale;
	}
	
}
void testApp:: guiEventTopCanvas(ofxUIEventArgs &e){
	
	string name = e.widget->getName();
	int kind = e.widget->getKind();
	
	if( name == "Remap"){
		ofxUIRangeSlider *rangeSlider = (ofxUIRangeSlider * ) e.widget;
		remapLowValue = rangeSlider->getScaledValueLow();
		remapHighValue = rangeSlider->getScaledValueHigh();
		rangeSlider->setValueHigh( remapHighValue );
		rangeSlider->setValueLow( remapLowValue );	
		
	}
	else if ( name =="Contrast stretch") {
		ofxUIToggle *toggle = (ofxUIToggle*) e.widget;
		contrastStretch = toggle->getValue();
		/*if ( toggle->getValue() == true) {
		 ofxUIRangeSlider *rangeSlider = toggle->getCanvasParent()->getWidget("Remap");
		 }*/
		cout << " In contrast stretch" << endl;
	}
	else if( name == "Reset"){
		remapLowValue = 0;
		remapHighValue = 255;
		ofxUICanvas * p =(ofxUICanvas*) ((e.widget)->getCanvasParent());
		ofxUIRangeSlider *r = (ofxUIRangeSlider *) p->getWidget("Remap");
		r->setValueLow( remapLowValue );
		r->setValueHigh( remapHighValue );
	}
	else if ( name == "THRES" ){
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		threshold = slider->getScaledValue();
	}
	
	
}

void testApp::guiEventCenterCanvas(ofxUIEventArgs &e ){
	// GUI EVENTS for center canvas
	string name = e.widget->getName(); 
	int kind = e.widget->getKind();
	
	if( name == "Area filter" ){
		ofxUIRangeSlider * rs = (ofxUIRangeSlider * ) e.widget;
		minBlobArea = rs->getScaledValueLow()*w_cam*h_cam;
		maxBlobArea = rs->getScaledValueHigh()*w_cam*h_cam;
	}
	
}
void testApp::guiEventOscCanvas(ofxUIEventArgs &e ){
	// GUI EVENTS for osc canvas
	string name = e.widget->getName(); 
	int kind = e.widget->getKind();
	
	if( name == "Send blob data through OSC"){
		ofxUIToggle *t = (ofxUIToggle *) e.widget;
		sendThroughOsc = ( t->getValue() );
	}
	else if ( name == "OSC HOST" ){
		ofxUITextInput *t = (ofxUITextInput * ) e.widget;
		HOST = t->getTextString();
		oscSender.setup(HOST , PORT);
	}
	else if ( name == "OSC PORT"){
		ofxUITextInput *t = (ofxUITextInput * ) e.widget;
		PORT = ofToInt( t->getTextString() );
		oscSender.setup(HOST , PORT);
	}
	
}
//--------------------------------------------------------------
void testApp::setupGUI(){
	//********** GUI *********** //
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
	uiWidth = 230;
	margenH = 0;
	topCanvasWidth = uiWidth;
	topCanvasHeight = 500;
	oscCanvasWidth = topCanvasWidth;
	oscCanvasHeight = 300;
	centerCanvasWidth = 400;
	centerCanvasHeight = 600;
	
	w_disp = 180;
	h_disp = int( h_cam * w_disp / float( w_cam ) );
	
	sliderWidth = 15;
	sliderHeight = h_disp; // Make sliders as tall as the images.
	
	
	red = 0; green = 1; blue = 2; grayscale = 3;
	selectedChannel = grayscale;
	
	
	leftCanvas = new ofxUICanvas(0.0, margenH, uiWidth , ofGetHeight() - margenH); 
	topCanvas = new ofxUICanvas( uiWidth + xInit, margenH, topCanvasWidth , topCanvasHeight); 
	centerCanvas = new ofxUICanvas( uiWidth + xInit + topCanvasWidth + xInit, margenH, centerCanvasWidth, centerCanvasHeight );
	oscCanvas = new ofxUICanvas( uiWidth + xInit, margenH + topCanvasHeight + xInit , oscCanvasWidth , oscCanvasHeight);
	
	// ********** LEFT CANVAS ********* //
	
	// SOURCE MODULE
	leftCanvas->addWidgetDown( new ofxUILabel("SOURCE", OFX_UI_FONT_SMALL) );
	leftCanvas->addSpacer();
	leftCanvas->addImage( "Untouched source" , &ofUntouchedSrc  );
	leftCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
	
	// Buttons for selecting source: Radio buttons for RGB
	vector <string> radioNames;
	radioNames.push_back("Red");
	radioNames.push_back("Green");
	radioNames.push_back("Blue");
	radioNames.push_back("Grayscale");
	
	leftCanvas->addRadio("Source selector" , radioNames , OFX_UI_ORIENTATION_VERTICAL);
	leftCanvas->addLabel("Selected source", OFX_UI_FONT_SMALL);
	leftCanvas->addSpacer();
	
	
	leftCanvas->addSlider("BLUR_S", 0, 10, sourceBlurFact , sliderWidth , sliderHeight );
	leftCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
	leftCanvas->addWidgetRight(new ofxUIImage( w_disp, h_disp , &ofSrc,"Source"), OFX_UI_ALIGN_RIGHT );
	
	
	// BACKGROUND MODULE
	leftCanvas->setWidgetPosition( OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_LEFT);
	leftCanvas->addWidgetDown( new ofxUILabel("BACKGROUND", OFX_UI_FONT_SMALL) );
	leftCanvas->addSpacer();
	leftCanvas->addSlider("BLUR", 0, 10, bkgBlurFact , sliderWidth , sliderHeight );
    leftCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
	leftCanvas->addWidgetRight(new ofxUIImage( w_disp, h_disp , &ofBkg ,"Background"), OFX_UI_ALIGN_RIGHT );
	
	// ********** SECOND CANVAS ********* //
	
	// SUBTRACTED MODULE
	topCanvas->setWidgetPosition( OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_LEFT);
	topCanvas->addWidgetDown( new ofxUILabel("SUBTRACTED", OFX_UI_FONT_SMALL) );
	topCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
	topCanvas->addSpacer();
	
	//topCanvas->addSlider("BLUR_S", 0, 10, sourceBlurFact , sliderWidth , sliderHeight );
	//topCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
	topCanvas->addRangeSlider("Remap", -255, 512, 0, 255, sliderWidth, sliderHeight );
	//topCanvas->setPlacer
	topCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
	topCanvas->addWidgetRight(new ofxUIImage( w_disp, h_disp, &ofSub, "Subtracted" ),OFX_UI_ALIGN_RIGHT );
	topCanvas->setWidgetPosition( OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_LEFT);
	topCanvas->setWidgetFontSize( OFX_UI_FONT_SMALL );
	topCanvas->addButton("Reset", false);
	topCanvas->addToggle("Contrast stretch", false);
	
	//topCanvas->addImage("Subtracted", &ofSub);//(new ofxUIImage( w_disp, h_disp , &ofSrc,"Source"), OFX_UI_ALIGN_RIGHT );
	
	
	// Threshold module
	topCanvas->setWidgetPosition( OFX_UI_WIDGET_POSITION_LEFT, OFX_UI_ALIGN_LEFT);
	topCanvas->addWidgetDown( new ofxUILabel("THRESHOLDED", OFX_UI_FONT_SMALL) );
	topCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
	topCanvas->addSpacer();
	
	topCanvas->addSlider("THRES", 0, 255, threshold , sliderWidth , sliderHeight );
	topCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
	topCanvas->addWidgetRight(new ofxUIImage( w_disp, h_disp , &ofThres,"Source"), OFX_UI_ALIGN_RIGHT );
	
	// ***** OSC CANVAS ******* //
	oscCanvas->addLabel("OSC",OFX_UI_FONT_SMALL);
	oscCanvas->addSpacer();
	oscCanvas->setWidgetFontSize( OFX_UI_FONT_SMALL );
	oscCanvas->addToggle("Send blob data through OSC", sendThroughOsc );
	oscCanvas->addLabel("Host", OFX_UI_FONT_SMALL);
	oscCanvas->addTextInput("OSC HOST", HOST);
	oscCanvas->addLabel("Port", OFX_UI_FONT_SMALL);
	oscCanvas->addTextInput("OSC PORT", ofToString(PORT));
	
	
	//********** CENTRAL CANVAS *************//
	
	centerCanvas->addLabel("BLOB DETECTION AND FILTERING", OFX_UI_FONT_SMALL);
	centerCanvas->addSpacer();
	centerCanvas->addImage("Blobs", &ofBlobs );
	centerCanvas->addRangeSlider("Area filter", 0, 0.5, 0, 0.5);
	
	
	ofAddListener( leftCanvas->newGUIEvent , this ,&testApp::guiEvent);	
	ofAddListener( topCanvas->newGUIEvent , this ,&testApp::guiEventTopCanvas);	
	ofAddListener( centerCanvas->newGUIEvent , this ,&testApp::guiEventCenterCanvas);
	ofAddListener( oscCanvas->newGUIEvent , this ,&testApp::guiEventOscCanvas);
	
}

//-------------------------------------------------------------
void testApp::draw(){
	//ofSetHexColor(0xffffff);
	
	
	// Contorno lib
	
	//labeledImage.draw( 300, 300) ;  
	
	
	// Blobs librer’a:
	//ofPushMatrix();
	//ofTranslate( margen_h + h_offset , margen_v + v_offset );
	/*for(int i = 0 ; i < blobs.size(); i++){
	 cout << "iterating over blobs. Size of vec: " << blobs.size() << "| i: " << i << endl;
	 CvBlob * blob = blobs.at(i);
	 ofCircle( blob->centroid.x , blob->centroid.y, 10 );		
	 }*/
	
	/*
	 for(CvBlobs::const_iterator it = blobs.begin() ; it !=blobs.end(); it++){
	 //cout << "iterating over blobs. Size of vec: " << blobs.size() << "| i: " << i << endl;
	 CvBlob * blob = it->second;
	 ofSetColor( 155, 40, 40);
	 ofFill();
	 ofCircle( it->second->centroid.x , it->second->centroid.y, 10 );	
	 //ofCircle( blob.centroid.x , blobs.centroid.y, 10 );	
	 
	 }*/
	
	//ofPopMatrix();
	//contourFinder.draw( margen_h + h_offset , margen_v + v_offset);
	
	
	/*for(int i = 0 ; i < contourFinder.nBlobs ;  i++){
	 ofxCvBlob blob = contourFinder.blobs.at(i);
	 
	 }*/
	//Letricas
	
}	

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if (key == ' '){
		bkg.setFromPixels( elVideo.getPixels() , w_cam, h_cam );
		ofBkg.setFromPixels( bkg.getPixelsRef() );
		//ofBkg.update();
	}
	if ( key == 'b') {
		blobDetectionOn = !blobDetectionOn;
	}
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	/*id++;
	 id = id % 4;
	 elVideo.setDeviceID( id );
	 cout << id << endl;
	 */
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
	
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
	
}
void testApp::exit()
{
    leftCanvas->saveSettings("GUI/guiSettings.xml");     
    delete leftCanvas; 
}