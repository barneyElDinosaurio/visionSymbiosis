#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofEnableSmoothing();
	id = 0;
	w_cam = 640/2;
	h_cam = 480/2;
	
	
	
	elVideo.listDevices();
	elVideo.setDeviceID(id);
	elVideo.initGrabber( w_cam , h_cam );
	
	colorImg.allocate( w_cam, h_cam );
	escalaDeGrises.allocate( w_cam, h_cam );
	imgRestada.allocate( w_cam, h_cam );
	bkg.allocate( w_cam, h_cam );
	bkgGrises.allocate( w_cam, h_cam );
	thresholded.allocate( w_cam, h_cam );
   
	// UI IMAGES
	ofUntouchedSrc.setFromPixels( colorImg.getPixelsRef() );
	ofBkg.setFromPixels( bkg.getPixelsRef() );
	ofSrc.setFromPixels( colorImg.getPixelsRef() );
	ofSub.setFromPixels( imgRestada.getPixelsRef() );
	ofThres.setFromPixels( thresholded.getPixelsRef() );

	// Parameters
	
	bkgBlurFact = 5;
	sourceBlurFact = 5;
	
	
	labeledImage.allocate( w_cam, h_cam );
	
	otherImage.allocate( w_cam, h_cam );
	
	
	font.loadFont("verdana.ttf", 14);
	
	
	threshold = 80;

	
	//********** GUI ***********
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
	uiWidth = 230;
	margenH = 0;
	
	w_disp = 180;
	h_disp = int( h_cam * w_disp / float( w_cam ) );
	
	sliderWidth = 15;
	sliderHeight = h_disp; // Make sliders as tall as the images.
	
	
	red = 0; green = 1; blue = 2; grayscale = 3;
	selectedChannel = grayscale;
	
		 
	leftCanvas = new ofxUICanvas(0.0, margenH, uiWidth , ofGetHeight() - margenH); 
	//leftCanvas->addWidget(<#ofxUIWidget *widget#>)
	

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
	
	

	// SUBTRACTED MODULE
	leftCanvas->setWidgetPosition( OFX_UI_WIDGET_POSITION_DOWN, OFX_UI_ALIGN_LEFT);
	leftCanvas->addWidgetDown( new ofxUILabel("SUBTRACTED", OFX_UI_FONT_SMALL) );
	leftCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
	leftCanvas->addSpacer();
	
	//leftCanvas->addSlider("BLUR_S", 0, 10, sourceBlurFact , sliderWidth , sliderHeight );
	//leftCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
	leftCanvas->addImage("Subtracted", &ofSub);//(new ofxUIImage( w_disp, h_disp , &ofSrc,"Source"), OFX_UI_ALIGN_RIGHT );
	
	
	// Threshold module
	leftCanvas->setWidgetPosition( OFX_UI_WIDGET_POSITION_LEFT, OFX_UI_ALIGN_LEFT);
	leftCanvas->addWidgetDown( new ofxUILabel("THRESHOLDED", OFX_UI_FONT_SMALL) );
	leftCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
	leftCanvas->addSpacer();
	
	leftCanvas->addSlider("THRES", 0, 255, threshold , sliderWidth , sliderHeight );
	leftCanvas->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
	leftCanvas->addWidgetRight(new ofxUIImage( w_disp, h_disp , &ofThres,"Source"), OFX_UI_ALIGN_RIGHT );
	
	
	

	
	ofAddListener( leftCanvas->newGUIEvent , this ,&testApp::guiEvent);	

	}


//--- GUI Event ----
void testApp::guiEvent( ofxUIEventArgs &e){
	
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
	else if ( name == "THRES" ){
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		threshold = slider->getScaledValue();
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
//--------------------------------------------------------------
void testApp::update(){

	elVideo.update();
	
	if( elVideo.isFrameNew() ){
		colorImg.setFromPixels( elVideo.getPixels(), w_cam, h_cam);
		ofUntouchedSrc.setFromPixels( colorImg.getPixelsRef() );
		
		// ************ PROCEDURE FOR BLOB DETECTION ******************
		
		// Select channel (selected by vRadio)
		if(selectedChannel == grayscale){
			escalaDeGrises = colorImg;
			bkgGrises = bkg;
			//cout << "In chaell selection if with selected Channel " << selectedChannel << endl;
		}
		else{
			//cout << "In chaell selection if" << endl;
			cout << "In chaell selection if with selected Channel " << selectedChannel << endl;
			colorImg.convertToGrayscalePlanarImage(escalaDeGrises, selectedChannel ) ;
			bkg.convertToGrayscalePlanarImage( bkgGrises, selectedChannel);
		}
		
		// Convert input and background to grayscale 
		
		//escalaDeGrises = colorImg;
		//bkgGrises = bkg;
		
		// Blur both of them
		
		bkgGrises.blur( bkgBlurFact ); // taking value from sliders!
		escalaDeGrises.blur( sourceBlurFact ); 
		/**/ ofBkg.setFromPixels( bkgGrises.getPixelsRef());
		/**/ ofSrc.setFromPixels( escalaDeGrises.getPixelsRef());
		
		// 
		imgRestada.absDiff( escalaDeGrises,  bkgGrises ); 
		/**/ ofSub.setFromPixels( imgRestada.getPixelsRef());
		
		thresholded = imgRestada;
		thresholded.threshold( threshold);
		/**/ ofThres.setFromPixels( thresholded.getPixelsRef() );
		

		IplImage *theImage = thresholded.getCvImage();
		IplImage *labelImage = cvCreateImage( cvGetSize(theImage) , IPL_DEPTH_LABEL, 1);
		IplImage *imprueba = cvCreateImage( cvGetSize(theImage) , 8, 3);
		
		 
		
		//contourFinder.findContours( thresholded , w_cam*h_cam/30.0f , w_cam*h_cam/4.0f , 5, false);
		//blobs.erase();
		unsigned int result = cvLabel(theImage , labelImage , blobs);
		/*cvRenderBlobs(labelImage, blobs, theImage, theImage, CV_BLOB_RENDER_CENTROID|
															CV_BLOB_RENDER_BOUNDING_BOX|
																CV_BLOB_RENDER_ANGLE|
																CV_BLOB_RENDER_COLOR);
		*/
		/*
		cout << "Depth of labelImage " << labelImage->depth << endl; 
		cout << "Depth of labeledImage (OFcv) " << labeledImage.getCvImage()->depth << endl;
		cout << "Length of blob vec " << blobs.size() << endl;
		*/
		cvRenderBlobs(labelImage, blobs, theImage, imprueba,  CV_BLOB_RENDER_CENTROID|
					  CV_BLOB_RENDER_BOUNDING_BOX|
					  CV_BLOB_RENDER_ANGLE|
					  CV_BLOB_RENDER_COLOR);

//		otherImage = labelImage.getPi
		labeledImage = imprueba;
		
		
		//ofxCvGrayscaleImage im = theImage;
		//im.draw( , <#float y#>, <#float w#>, <#float h#>)
		
		
		//imshow("trin",labelImage);
	}
	

}

//-------------------------------------------------------------
void testApp::draw(){
	ofSetHexColor(0xffffff);
	
	
	// Contorno lib
	
	labeledImage.draw( 300, 120) ;  

	
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
	if (key == '.'){
		threshold++;
		cout << "threshold " << threshold << endl;
	}
	if (key == ','){
		threshold--;
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