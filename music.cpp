#include <windows.h>  
#include <mmsystem.h>
#include <mmdeviceapi.h>  
#include <endpointvolume.h>
#include <audioclient.h>  
#include <endpointvolume.h>
#include<cmath>
#include <fftw3.h>
#include <thread>
#include<iostream>
#include<GL/glut.h>

using namespace std;

bool keepgoing=true;
IMMDeviceEnumerator *deviceEnumerator =NULL;
IAudioEndpointVolume* pVolume=NULL;	
IAudioCaptureClient* cc = nullptr;
IAudioClient *ac=NULL;
float amplitude=0,frequency,tranfreq=0;	
IMMDevice *device = NULL;
UINT32 sampleRate = 0;
GLsizei winwidth=400,winheight=400;
class screenpt{
public:float x,y;
};
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
	screenpt hexvertex,circtr;
	circtr.x=0;
	circtr.y=0;
	GLdouble theta,freqtheta=frequency*M_PI/20000;
	GLint k;

	glColor4f(0.0, 0.0, 0.0,1.0);
	for(k=0;k<5000;k++)
	{
		theta=2*M_PI*(float)k/5000.0f;
		hexvertex.x=(circtr.x+0.7*cos(theta));
        hexvertex.y=(circtr.y+0.7*sin(theta));
		glBegin(GL_LINE_LOOP);
	    glVertex2f(circtr.x,circtr.y);
        glVertex2f(hexvertex.x+0.02*(cos(theta))*sin(k*frequency/70000),hexvertex.y+0.02*(sin(theta))*sin(k*frequency/70000));
		glEnd();
	}
	float lastx=0,lasty=0;

	for(int i = 1; i <log((amplitude)*10000000)/log(5);i=i+1){
	glColor4f(0.678-0.070*(i-1), 0.847-0.070*(i-1), 0.902-0.070*(i-1),1.0);
		glBegin(GL_LINE_LOOP);
		for(k=83;k<383;k++){
			theta=2*M_PI*(float)k/1000.0f;
			hexvertex.x=(circtr.x+(0.1+0.5*i/10)*cos(theta));
	        hexvertex.y=(circtr.y+(0.1+0.5*i/10)*sin(theta));
        	glVertex2f(hexvertex.x,hexvertex.y);
		}
		glEnd();
		glBegin(GL_LINE_LOOP);
		for(k=416;k<716;k++){
			theta=2*M_PI*(float)k/1000.0f;
			hexvertex.x=(circtr.x+(0.1+0.5*i/10)*cos(theta));
	        hexvertex.y=(circtr.y+(0.1+0.5*i/10)*sin(theta));
        	glVertex2f(hexvertex.x,hexvertex.y);
		}
		glEnd();
		glBegin(GL_LINE_LOOP);
		for(k=749;k<1049;k++){
			theta=2*M_PI*(float)k/1000.0f;
			hexvertex.x=(circtr.x+(0.1+0.5*i/10)*cos(theta));
	        hexvertex.y=(circtr.y+(0.1+0.5*i/10)*sin(theta));
        	glVertex2f(hexvertex.x,hexvertex.y);
		}
		glEnd();
	}
	glFlush();
}
void winReshapeFcn(int newwidth,int newheight){
	glViewport(0,0,(GLsizei)newwidth,(GLsizei)newheight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(newheight>=newwidth){
		gluOrtho2D(-1.0,+1.0,-(GLdouble)newheight/(GLdouble)newwidth,(GLdouble)newheight/(GLdouble)newwidth);
	}
	else{
		gluOrtho2D(-(GLdouble)newwidth/(GLdouble)newheight,+(GLdouble)newwidth/(GLdouble)newheight,-1.0,1.0);
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glutPostRedisplay();
	glFlush();
}

void initAudio(){
	HRESULT hr = CoInitialize(nullptr);
	if(FAILED(hr))
		exit(3);
	hr= CoCreateInstance(__uuidof(MMDeviceEnumerator),NULL,CLSCTX_ALL,__uuidof(IMMDeviceEnumerator),(void**)&deviceEnumerator);
	if(FAILED(hr))
		exit(3);
	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender,eConsole,&device);
	if(FAILED(hr))
		exit(3);
	hr=device->Activate(__uuidof(IAudioClient),CLSCTX_ALL,NULL,(void**)&ac);
	if(FAILED(hr))
		exit(3);
	WAVEFORMATEX* waveFormat = nullptr;
	hr= ac->GetMixFormat(&waveFormat);
	if(FAILED(hr))
		exit(3);
	sampleRate= waveFormat->nSamplesPerSec;
	//waveFormat->nChannels=1;
	hr=ac->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0 , 0, waveFormat, nullptr);
	if(FAILED(hr))
		exit(3);
	hr=ac->GetService(__uuidof(IAudioCaptureClient), (void**)&cc);
	if(FAILED(hr))
		exit(3);
	hr = ac->Start();
	if(FAILED(hr))
		exit(3);

    hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&pVolume);
    if(FAILED(hr))
		exit(3);
}


void getstuff(){

	BYTE *data = nullptr;
	UINT32 packetLength = 0;
	DWORD flags = 0;
	UINT64 devicePosition=0;
	UINT64 qpcPosition=0;
	while(keepgoing){
		try{
			Sleep(1);
		    float volumeLevel = 0.0f;
		    HRESULT hr = pVolume->GetMasterVolumeLevelScalar(&volumeLevel);
			hr = cc->GetBuffer(&data, &packetLength, &flags, &devicePosition, &qpcPosition);        
			float* pSamples = reinterpret_cast<float*>(data);
			if (packetLength!=0){
				float* samples = (float*)fftwf_malloc(sizeof(float) * packetLength/2);

				amplitude = 0.0f; 

				for (UINT32 i = 1; i < packetLength/2; ++i) {
					float leftChannel = pSamples[2 * i];
		            float rightChannel = pSamples[2 * i + 1];
		            float monoSample = (leftChannel + rightChannel) / 2.0f;
		            samples[i]=monoSample;
		            amplitude +=fabs(monoSample);
		        }
				amplitude /= (float)packetLength;
				amplitude*=volumeLevel;
				float maxfreq = 0.0f;
				float totalPower = 0.0f;
				fftwf_complex* fftResult = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * (packetLength/4));
				fftwf_plan plan = fftwf_plan_dft_r2c_1d((int)packetLength/2,samples, fftResult, FFTW_ESTIMATE);
				fftwf_execute(plan);
				fftwf_destroy_plan(plan);
				float powerSpectrum=0.0f;
				UINT32 i = 0;
				for (i = 0; i < packetLength/4; ++i) {
				    float re = fftResult[i][0];
				    float im = fftResult[i][1];
				    powerSpectrum = sqrt(re * re + im * im);
					if (maxfreq<powerSpectrum){
						maxfreq=powerSpectrum;
						totalPower=i;
					}
				}
				frequency=(((double)sampleRate)*(double)totalPower*2/(double)packetLength);
				glutPostRedisplay();
			}
			hr=cc->ReleaseBuffer(packetLength);
			if(FAILED(hr))
				cout<<"Fail itseems"<<endl;
		}
		catch(...){
			exit(1);
		}
	}
	exit(1);
}


int main(int argc,char **argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(winwidth,winheight);
	glutCreateWindow("MUSIC VISUALIZER");
	initAudio();
	thread t1(getstuff);
	glutDisplayFunc(display);
	glutReshapeFunc(winReshapeFcn);
	glClearColor(0.0,0.75,0.5,1.0);
	glutMainLoop();
	keepgoing=false;
	t1.join();
	pVolume->Release();
	ac->Stop();
	ac->Release();
	deviceEnumerator->Release();
	device->Release();
	CoUninitialize();
	exit(1);
}

class Solution {
    public int countOfSubstrings(String s) {
        int a = -1;
        int e = -1;
        int i = -1;
        int o = -1;
        int u = -1;
        int beg=0,end=-1,res=0,lastbeg=0,temp=0; 
        Boolean one=false;    
        for (int i =0;i<s.length();i++){
            while ((a<beg || a>end) || (e<beg || e>end) || (i<beg || i>end)||  (o<beg || o>end)||  (u<beg || u>end)){
                end+=1;
                if (end==s.length()){
                    if (one==false){return res;}
                    return res+1;
                    }
                if (s.charAt(end)=='a'){
                    if (a>=beg && a<=end){
                        temp=beg;
                        beg=Math.min(e,i,o,u);
                        beg=Math.max(temp,beg);
                    }
                    a=end;
                }
                if (s.charAt(end)=='e'){
                    if (e>=beg && e<=end){
                        temp=beg;
                        beg=Math.min(a,i,o,u);
                        beg=Math.max(temp,beg);
                    }
                    e=end;
                }
                if (s.charAt(end)=='i'){
                    if (i>=beg && i<=end){
                        temp=beg;
                        beg=Math.min(a,e,o,u);
                        beg=Math.max(temp,beg);
                    }
                    i=end;
                }
                if (s.charAt(end)=='o'){
                    if (o>=beg && o<=end){
                        temp=beg;
                        beg=Math.min(a,e,i,u);
                        beg=Math.max(temp,beg);
                    }
                    o=end;
                }
                if (s.charAt(end)=='u'){
                    if (u>=beg && u<=end){
                        temp=beg;
                        beg=Math.min(a,e,i,o);
                        beg=Math.max(temp,beg);
                    }
                    u=end;
                }
            }
        res+=s.length()-(end-beg+1)-lastbeg;
        one=true;
        lastbeg=beg;
        beg+=1;
        
        }
    if (one==false){return res;}
    return res+1;
    }
}