#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "cvblob.h"
#include "ofxUI.h"
#include "ofxOsc.h"

using namespace cvb;
using namespace cv;

class testApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void exit();
	
	// Videodude
	
	ofVideoGrabber elVideo;
	
	//************* ofxUI *****************//
	
	ofxUICanvas * leftCanvas;
	ofxUICanvas * topCanvas;
	ofxUICanvas * centerCanvas;
	ofxUICanvas * oscCanvas;
	void guiEvent(ofxUIEventArgs &e);
	void guiEventTopCanvas(ofxUIEventArgs &e);
	void guiEventCenterCanvas(ofxUIEventArgs &e);
	void guiEventOscCanvas(ofxUIEventArgs &e);
	
	// Parameters
	int uiWidth;
	int margenH;
	int sliderWidth;
	int sliderHeight;
	int topCanvasWidth;
	int topCanvasHeight;
	int centerCanvasWidth;
	int centerCanvasHeight;
	int oscCanvasWidth;
	int oscCanvasHeight;
	
	void setupGUI();
	
	// Dimensiones de las im‡genes a mostrar 
	int w_disp;
	int h_disp;
	
	// Canales
	int selectedChannel;
	int red, blue, green, grayscale;
	
	// *******   OSC ******* //
	
	ofxOscSender oscSender;
	bool sendThroughOsc;
	string HOST;
	int PORT;
	
	 
	// *************    CV   *************** //
	
	// Imagen fuente
	ofxCvColorImage colorImg;
	
	// Background para restar
	ofxCvColorImage bkg;
	
	// Imagen fuente en grises
	ofxCvGrayscaleImage escalaDeGrises;
	ofxCvGrayscaleImage imgRestada;
	ofxCvGrayscaleImage bkgGrises;
	ofxCvGrayscaleImage thresholded;
	
	ofxCvColorImage labeledBlobsImage;
	ofxCvColorImage otherImage;
	
	// Im‡genes para display
	ofImage ofUntouchedSrc;
	ofImage ofSrc;
	ofImage ofBkg;
	ofImage ofSub;
	ofImage ofThres;
	ofImage ofBlobs;
	
	ofImage laPruebaExterna;
	
	bool blobDetectionOn;
	
	// Ipl Images for cvBlob
	IplImage *thresholdedIpl;
//	IplImage *labelImage;
	//IplImage *iplBlobsImage;
	
	
	//cvBlob stuff	
	CvBlobs blobs;
	
	// Parameters
	int bkgBlurFact;
	int sourceBlurFact;
	int remapLowValue;
	int remapHighValue;
	bool contrastStretch;
	float minBlobArea;
	float maxBlobArea;
//	std::map<CvLabel,CvBlob *> CvBlobs;

	
	int threshold;
	// Dimensiones de la captura
	int w_cam;
	int h_cam; 
	
	
	
	int id;
	

};
