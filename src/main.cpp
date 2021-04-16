// DarkHelp then pulls in OpenCV and much more, so this keeps the headers simple.
#include <DarkHelp.hpp>


const std::string darkplate_configuration	= "DarkPlate.cfg";
const std::string darkplate_best_weights	= "DarkPlate_best.weights";
const std::string darkplate_names			= "DarkPlate.names";
const size_t class_plate					= 0;
cv::Size network_size;


void process_plate(DarkHelp & darkhelp, cv::Mat plate)
{
	auto results = darkhelp.predict(plate);
	if (results.empty())
	{
		// nothing we can do with this image since no license plate was found
		std::cout << "-> failed find a plate in this RoI" << std::endl;
		return;
	}

	// sort the results from left-to-right based on the mid-x point of each detected object
	std::sort(results.begin(), results.end(),
			[](const DarkHelp::PredictionResult & lhs, const DarkHelp::PredictionResult & rhs)
			{
				// put the "license plate" class first so the characters get draw overtop of this class
				if (lhs.best_class == class_plate)	return true;
				if (rhs.best_class == class_plate)	return false;

				// otherwise, sort by the horizontal coordinate
				// (this obviously only works with license plates that consist of a single row of characters)
				return lhs.original_point.x < rhs.original_point.x;
			});

	std::cout << "-> results: " << results << std::endl;

	std::string license_plate;
	double probability = 0.0;
	for (const auto prediction : results)
	{
		probability += prediction.best_probability;
		if (prediction.best_class != class_plate)
		{
			license_plate += darkhelp.names[prediction.best_class];
		}
	}
	std::cout << "-> license plate: \"" << license_plate << "\", score: " << std::round(100.0 * probability / results.size()) << "%" << std::endl;

	// store the sorted results back in DarkHelp so the annotations are drawn with the license plate first
	darkhelp.prediction_results = results;
	cv::Mat mat = darkhelp.annotate();
	mat.copyTo(plate);

	cv::imshow("plate", plate);
	cv::waitKey(1);

	return;
}


void process_plate(DarkHelp & darkhelp, cv::Mat frame, const DarkHelp::PredictionResult & prediction)
{
	cv::Rect roi = prediction.rect;

	if (roi.width < 1 or roi.height < 1)
	{
		std::cout << "-> ignoring impossibly small plate (x=" << roi.x << " y=" << roi.y << " w=" << roi.width << " h=" << roi.height << ")" << std::endl;
		return;
	}

	// increase the RoI to match the network dimensions, but stay within the bounds of the frame
	if (roi.width >= network_size.width or roi.height >= network_size.height)
	{
		// something is wrong with this plate, since it seems to be the same size or bigger than the original frame size!
		std::cout << "-> ignoring too-big plate (x=" << roi.x << " y=" << roi.y << " w=" << roi.width << " h=" << roi.height << ")" << std::endl;
		return;
	}

	const double dx = 0.5 * (network_size.width		- roi.width	);
	const double dy = 0.5 * (network_size.height	- roi.height);

	roi.x		-= std::floor(dx);
	roi.y		-= std::floor(dy);
	roi.width	+= std::ceil(dx * 2.0);
	roi.height	+= std::ceil(dy * 2.0);

	// check all the edges and reposition the RoI if necessary
	if (roi.x < 0)							roi.x = 0;
	if (roi.y < 0)							roi.y = 0;
	if (roi.x + roi.width	> frame.cols)	roi.x = frame.cols - roi.width;
	if (roi.y + roi.height	> frame.rows)	roi.y = frame.rows - roi.height;

	#if 0
	std::cout	<< "-> plate found: " << prediction << std::endl
				<< "-> roi: x=" << roi.x << " y=" << roi.y << " w=" << roi.width << " h=" << roi.height << std::endl;
	#endif

	// the RoI should now be the same size as the network dimensions, and all edges should be valid

	cv::Mat plate = frame(roi);
	process_plate(darkhelp, plate);

	return;
}


void process_frame(DarkHelp & darkhelp, cv::Mat frame)
{
	// we need to find all the license plates in the image
	auto result = darkhelp.predict(frame);

	for (const auto & prediction : result)
	{
		// at this stage we're only interested in the "license plate" class, ignore everything else
		if (prediction.best_class == class_plate)
		{
			process_plate(darkhelp, frame, prediction);
		}
	}

	return;
}


void process(DarkHelp & darkhelp, const std::string & filename)
{
	std::cout << "Processing video file \"" << filename << "\"" << std::endl;

	std::string basename = filename;
	size_t p = basename.rfind("/");
	if (p != std::string::npos)
	{
		basename.erase(0, p + 1);
	}
	p = basename.rfind(".");
	if (p != std::string::npos)
	{
		basename.erase(p);
	}

	cv::VideoCapture cap(filename);
	if (cap.isOpened() == false)
	{
		std::cout << "ERROR: \"" << filename << "\" is not a valid video file, or perhaps does not exist?" << std::endl;
		return;
	}

	const double width		= cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH	);
	const double height		= cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT	);
	const double frames		= cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_COUNT	);
	const double fps		= cap.get(cv::VideoCaptureProperties::CAP_PROP_FPS			);
	const size_t round_fps	= std::round(fps);

	std::cout	<< "-> " << static_cast<size_t>(width) << " x " << static_cast<size_t>(height) << " @ " << fps << " FPS" << std::endl
				<< "-> " << frames << " frames (" << static_cast<size_t>(std::round(frames / fps)) << " seconds)" << std::endl;

	cv::VideoWriter output;
	output.open(basename + "_output.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(width, height));

	size_t frame_counter = 0;
	while (true)
	{
		cv::Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			break;
		}

		if (frame_counter % round_fps == 0)
		{
			std::cout << "\r-> frame #" << frame_counter << " (" << std::round(100 * frame_counter / frames) << "%)" << std::flush;
		}

		process_frame(darkhelp, frame);

		output.write(frame);

		frame_counter ++;
	}
	std::cout << "\r-> done processing " << frame_counter << " frames from " << filename << std::endl;

	return;
}


int main(int argc, char *argv[])
{
	try
	{
		DarkHelp darkhelp;

		// first thing we need to do is find the neural network
		bool initialization_done = false;
		for (const auto path : {"./", "../", "../../", "nn/", "../nn/", "../../nn/"})
		{
			const auto fn = path + darkplate_configuration;
			std::cout << "Looking for " << fn << std::endl;
			std::ifstream ifs(fn);
			if (ifs.is_open())
			{
				ifs.close();
				std::cout << "Found neural network: " << fn << std::endl;
				const std::string cfg		= fn;
				const std::string names		= path + darkplate_names;
				const std::string weights	= path + darkplate_best_weights;
				darkhelp.init(cfg, weights, names);
				darkhelp.annotation_auto_hide_labels	= false;
				darkhelp.annotation_include_duration	= false;
				darkhelp.annotation_include_timestamp	= false;
				darkhelp.enable_tiles					= true;
				darkhelp.combine_tile_predictions		= true;
				darkhelp.include_all_names				= true;
				darkhelp.names_include_percentage		= true;
				darkhelp.threshold						= 0.25;
				darkhelp.sort_predictions				= DarkHelp::ESort::kUnsorted;
				initialization_done						= true;
				break;
			}
		}
		if (initialization_done == false)
		{
			throw std::runtime_error("failed to find the neural network DarkPlate.cfg");
		}

		// remember the size of the network, since we'll need to crop plates to this exact size
		network_size = darkhelp.network_size();

		for (int idx = 1; idx < argc; idx ++)
		{
			process(darkhelp, argv[idx]);
		}
	}
	catch (const std::exception & e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cout << "ERROR: unknown exception" << std::endl;
		return 2;
	}

	return 0;
}
