#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "cvblob.h"

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
	
		// Videodude
		
	ofVideoGrabber elVideo;
	
	//CV - cada una de las im‡genes.
	ofxCvColorImage colorImg;
	
	// Background para restar
	ofxCvColorImage bkg;
	
	ofxCvGrayscaleImage red;
	ofxCvGrayscaleImage blue;
	ofxCvGrayscaleImage escalaDeGrises;
	ofxCvGrayscaleImage imgRestada;
	ofxCvGrayscaleImage bkgGrises;
	ofxCvGrayscaleImage thresholded;
	
	ofxCvColorImage labeledImage;
	ofxCvColorImage otherImage;
	
	
	ofxCvContourFinder contourFinder;
	
	//cvBlob stuff	
	CvBlobs blobs;
	
//	std::map<CvLabel,CvBlob *> CvBlobs;

	
	int threshold;
	// Dimensiones de la captura
	int w_cam;
	int h_cam; 
	
	// Dimensiones de las im‡genes
	int w_disp;
	int h_disp;
	
	int id;
	
	// Layout
	int h_offset;
	int v_offset;
	int margen_h;
	int margen_v;
	
	// letra
	ofTrueTypeFont font;
};
