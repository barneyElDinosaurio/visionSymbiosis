#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	id = 0;
	w_cam = 640/2;
	h_cam = 480/2;
	
	w_disp = 320;
	h_disp = 240;
	elVideo.listDevices();
	elVideo.setDeviceID(id);
	elVideo.initGrabber( w_cam , h_cam );
	
	colorImg.allocate( w_cam, h_cam );
	escalaDeGrises.allocate( w_cam, h_cam );
	imgRestada.allocate( w_cam, h_cam );
	bkg.allocate( w_cam, h_cam );
	bkgGrises.allocate( w_cam, h_cam );
	thresholded.allocate( w_cam, h_cam );
	
	labeledImage.allocate( w_cam, h_cam );
	
	otherImage.allocate( w_cam, h_cam );
	
	
	font.loadFont("verdana.ttf", 14);
	
	
	threshold = 80;
	
	// Layout
	margen_h = 20;
	margen_v = 40;
	h_offset = 340;
	v_offset = 300;
	
//	namedWindow("LA ventana" , 0x00000001 );
	
	}

//--------------------------------------------------------------
void testApp::update(){

	elVideo.update();
	
	if( elVideo.isFrameNew() ){
		colorImg.setFromPixels( elVideo.getPixels(), w_cam, h_cam);
		escalaDeGrises = colorImg;
		bkgGrises = bkg;
		bkgGrises.blurHeavily();
		escalaDeGrises.blurHeavily();
		
		imgRestada.absDiff( escalaDeGrises,  bkgGrises ); 
		
		thresholded = imgRestada;
		thresholded.threshold( threshold);
		

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
		
		cout << "Depth of labelImage " << labelImage->depth << endl; 
		cout << "Depth of labeledImage (OFcv) " << labeledImage.getCvImage()->depth << endl;
		cout << "Length of blob vec " << blobs.size() << endl;
		
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
	
	
	//Captura
	colorImg.draw( margen_h, margen_v, w_disp , h_disp);
	
	//Fondo
	bkg.draw( margen_h, margen_v + v_offset , w_disp , h_disp);
	
	// Restada
	imgRestada.draw( margen_h + h_offset , margen_v, w_disp , h_disp);
	
	// con Umbral
	thresholded.draw( margen_h + h_offset , margen_v + v_offset, w_disp , h_disp);
	
	// Contorno lib
	
	labeledImage.draw( margen_h + 2*h_offset , margen_v + v_offset) ;  

	
	// Blobs librer’a:
	ofPushMatrix();
	ofTranslate( margen_h + h_offset , margen_v + v_offset );
	/*for(int i = 0 ; i < blobs.size(); i++){
		cout << "iterating over blobs. Size of vec: " << blobs.size() << "| i: " << i << endl;
		CvBlob * blob = blobs.at(i);
		ofCircle( blob->centroid.x , blob->centroid.y, 10 );		
	}*/
	
	for(CvBlobs::const_iterator it = blobs.begin() ; it !=blobs.end(); it++){
		//cout << "iterating over blobs. Size of vec: " << blobs.size() << "| i: " << i << endl;
		CvBlob * blob = it->second;
		ofSetColor( 155, 40, 40);
		ofFill();
		ofCircle( it->second->centroid.x , it->second->centroid.y, 10 );	
		//ofCircle( blob.centroid.x , blobs.centroid.y, 10 );	
		
	}
	ofPopMatrix();
	//contourFinder.draw( margen_h + h_offset , margen_v + v_offset);
	
		
	/*for(int i = 0 ; i < contourFinder.nBlobs ;  i++){
		ofxCvBlob blob = contourFinder.blobs.at(i);
		
	}*/
	//Letricas
	ofSetColor(255);
	font.drawString( "CAMARA", margen_h , 35);
	font.drawString( "FONDO", margen_h , 35 + v_offset );
	font.drawString( "RESTADA", margen_h + h_offset , 35);
	font.drawString( "+THRESHOLD", margen_h + h_offset , 35 + v_offset);
//	font.drawString( , <#float x#>, <#float y#>)

}	

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if (key == ' '){
		bkg.setFromPixels( elVideo.getPixels() , w_cam, h_cam );
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