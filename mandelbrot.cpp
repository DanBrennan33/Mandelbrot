// To compile: /usr/local/gcc/gcc-cilk/bin/g++ -std=c++11 -pthread mandelbrot.cpp TP.cpp

#include <iostream>
#include <fstream>
#include <complex> // google this
#include <vector>  // thread table
#include <thread>
//#include <stdlib.h>
#include "TP.h"

using namespace std;

class Pixel {
	uint8_t R;
	uint8_t G;
	uint8_t B;
	public:
	Pixel() {}
	Pixel(uint8_t r, uint8_t g, uint8_t b) : R(r), G(g), B(b) {}
	uint8_t getR() const { return R; }
	uint8_t getG() const { return G; }
	uint8_t getB() const { return B; }
};

class Image {
	const size_t W;
	const size_t H;
	Pixel* data;
	public:
	Image() : W(0), H(0), data(nullptr) {}
	Image(size_t w, size_t h) : W(w), H(h), data(new Pixel[W*H]) {}
	~Image() { if(data) delete [] data; }
	size_t getW() { return W; }
	size_t getH() { return H; }
	void SetPix(size_t x, size_t y, Pixel& pix) { data[y*W+ x] = pix; }
	Pixel& GetPix(size_t x, size_t y) const { return data[y*W + x]; }
	void WritePPM_P3(string fn) const {
		fstream os(fn.c_str(), ios::out);
		if(os.is_open()) {
			os << "P3\n"
			   << W << " " << H << "\n"
			   << "255\n";
			for(size_t y = 0; y < H; y++) {
				for(size_t x = 0; x < W; x++) {
					Pixel pix = GetPix(x,y);
					os << (int) pix.getR() << " " 
					   << (int) pix.getG() << " " 
					   << (int) pix.getB() << " ";
				}
			}
			os.close();
		} else throw string("Cannot create file ") + fn;
	}
	void WritePPM_P6(string fn) const {
		fstream os(fn.c_str(), ios::out | ios::binary);
		if(os.is_open()) {
			os << "P6\n"
			   << W << " " << H << "\n"
			   << "255\n";
			for(size_t y = 0; y < H; y++) {
				for(size_t x = 0; x < W; x++) {
					Pixel pix = GetPix(x,y);
					os << pix.getR()  
					   << pix.getG()  
					   << pix.getB();
				}
			}
			os.close();
		} else throw string("Cannot create file ") + fn;
	}
};

class Mandelbrot {
	int maxIter;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	
	public:
	Mandelbrot(int _maxIter, double _xmin, double _xmax, double _ymin, double _ymax) 
	: maxIter(_maxIter), 
	  xmin(_xmin), xmax(_xmax), 
	  ymin(_ymin), ymax(_ymax) 
	{}
	void RunXXX(Image& image) {
		size_t const W = image.getW();
		size_t const H = image.getH();
//		uint64_t iterTotal = 0LL;
		for (size_t x = 0; x < W; x++)
			for (size_t y = 0; y < H; y++) {
	  			complex<double> c(xmin + x/(W-1.0)*(xmax-xmin), ymin + y/(H-1.0)*(ymax-ymin));
				complex<double> z = 0;
				int iterations;
	        		for (iterations = 0; iterations < maxIter && abs(z) < 2.0; ++iterations) 
				z = z*z + c;
			//	iterTotal += iterations;

				uint32_t color = double(iterations) / double(maxIter) * (1 << 24) ;

				uint8_t r = color 		&0xff;
				uint8_t g = (color >> 8) 	&0xff;
				uint8_t b = (color >> 8) 	&0xff;

				Pixel pixcolor(r,g,b);
				image.SetPix(x, y, pixcolor);
				// TODO: image[x][y] = (iterations == maxIter) ? set_color : non_set_color;
			}
//		cout << "Total iterations " << iterTotal << "\n";
	}
	void doWork(Image& image, size_t start, size_t end) {
//		Timer t;

//		t.Start();
		size_t const W = image.getW();
		size_t const H = image.getH();
//		uint64_t iterTotal = 0LL;
		for (size_t y = start; y < end; y++) { 		// for each row
			for (size_t x = 0; x < W; x++) {	// for each column
	  			complex<double> c(xmin + x/(W-1.0)*(xmax-xmin), ymin + y/(H-1.0)*(ymax-ymin));
				complex<double> z = 0;
				int iterations;
//	        		for (iterations = 0; iterations < maxIter && abs(z) < 2.0; ++iterations) 
	        		for (iterations = 0; iterations < maxIter && norm(z) < 4.0; ++iterations) 
				z = z*z + c;
//				iterTotal += iterations;

				uint32_t color = double(iterations) / double(maxIter) * (1 << 24) ;

				uint8_t r = color 		&0xff;
				uint8_t g = (color >> 8) 	&0xff;
				uint8_t b = (color >> 16) 	&0xff;

				Pixel pixcolor(r,g,b);
				image.SetPix(x, y, pixcolor);
				// TODO: image[x][y] = (iterations == maxIter) ? set_color : non_set_color;
			}
		}		
//		cout << "Total iterations " << iterTotal << "\n";
	//	t.Stop();
	//	std::cout << "dWork: stat, end = " << start << end << " " << (double)t.usecs()/1000000 << " seconds.\n";
	}
	void Run(Image& img) {
		size_t const H = img.getH();
		doWork(img,0,H);
	}
	void RunThreaded(Image& img) {		
		size_t const H = img.getH();
		
		int NUM_THREADS = std::thread::hardware_concurrency();
		//std::cout << "running with " << NUM_THREADS << " threads\n";
		//if(NUM_THREADS == 2) NUM_THREADS = 4;
		std::cout << "running with " << NUM_THREADS << " threads\n";
		std::vector<std::thread> t(NUM_THREADS);

		size_t start = 0;
		size_t end   = H;

		size_t chunk = (end - start + (NUM_THREADS-1)) / NUM_THREADS;

		for(int i = 0; i < NUM_THREADS; i++) {
			size_t s = start + i * chunk;
			size_t e = s + chunk;
		        if(i == NUM_THREADS - 1) e = end;

		#if 1 // call thread directly

		// Note how to start a thread with a class member function.
		t[i] = std::thread (&Mandelbrot::doWork, this, ref(img), s, e);
		#else // call thread with a functional pointer
		//std::function<void(void)> b = std::bind( &Application::doWork, this, s, e);
		// auto b = std::bind( &Application::doWork, this, s, e, needLock); // also works...
		//t[i] = std::thread ( b );
		#endif
		}
		for( auto& e : t ) e.join();
	}
	void RunThreadPool(Image& img) {
		size_t const H = img.getH();
		ThreadPool tp;
		for(size_t y = 0; y < H; y++) {
			auto job = std::bind( &Mandelbrot::doWork, this, ref(img), y, y+1);
			tp.AddJob( job );
			// tp.AddJob( auto job = std::bind( &Mandelbrot::doWork, this, ref(img), y, y+1) );
		}
		tp.ShutDown();
	}
};		


int main(int argc, char **argv) {
	Timer t;
	// image parameters
	string fn = string(argv[0]) + ".ppm";
	size_t W	= 640;
	size_t H	= 480;
	
	// Mandelbrot parameters
	int maxIter	= 200;
	double xmin 	= -2;
	double xmax	= 1;
	double ymin	= -2;
	double ymax	= 2;

	if(argc == 9) { 
		//image parameters
		fn 	= argv[1];
		W	= atoi(argv[2]);
		H	= atoi(argv[3]);
		
		//mandelbrot parameters
		maxIter 	= atoi(argv[4]);
		xmin 		= atof(argv[5]);
		xmax		= atof(argv[6]);
		ymin		= atof(argv[7]);
		ymax		= atof(argv[8]);
	}
		Image img(W,H);
		Mandelbrot m( maxIter, xmin, xmax, ymin, ymax);
		
		t.Start();
		m.Run(img);
		t.Stop();
		std::cout << "Time without threads: " << (double)t.usecs()/1000000 << "\n";
//		img.WritePPM_P3(fn + "P3.ppm"); 		//test - OK
		img.WritePPM_P6(fn + "P6.ppm"); 		//test - OK
		
		t.Start();
		m.RunThreaded(img);
		t.Stop();
		std::cout << "Time with threads: " << (double)t.usecs()/1000000 << "\n";
		img.WritePPM_P6(fn + "threaded.P6.ppm");	//test - OK

		t.Start();
		m.RunThreadPool(img);
		t.Stop();
		std::cout << "Time with threadpool: " << (double)t.usecs()/1000000 << "\n"; 
		img.WritePPM_P6(fn + "threadpool.P6.ppm"); 	//test - OK
}
