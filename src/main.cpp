#include <opencv2/core.hpp>

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/mcc.hpp>
#include <opencv2/mcc/ccm.hpp>
#include <iostream>

using namespace std;
using namespace cv;
using namespace mcc;
using namespace ccm;
using namespace std;

const char *about = "Basic chart detection";
const char *keys = {
    "{ help h usage ? |    | show this message }"
    "{t        |      |  chartType: 0-Standard, 1-DigitalSG, 2-Vinyl }"
    "{v        |      | Input from video file, if ommited, input comes from camera }"
    "{ci       | 0    | Camera id if input doesnt come from video (-v) }"
    "{f        | 1    | Path of the file to process (-v) }"
    "{nc       | 1    | Maximum number of charts in the image }"};


float CChartVinylColors[18][9] = {
    //       sRGB              CIE L*a*b*             Munsell Notation
    // ---------------  ------------------------     Hue Value / Chroma
    // R     G      B        L*      a*       b*
    {255.0f, 255.0f, 255.0f, 100.0f, 0.0052f, -0.0104f, -1.0f, -1.0f, -1.0f},
    {176.0f, 180.0f, 183.0f, 73.0834f, -0.820f, -2.021f, -1.0f, -1.0f, -1.0f},
    {150.0f, 151.0f, 155.0f, 62.493f, 0.426f, -2.231f, -1.0f, -1.0f, -1.0f},
    {119.0f, 120.0f, 124.0f, 50.464f, 0.447f, -2.324f, -1.0f, -1.0f, -1.0f},
    {88.0f, 89.0f, 91.0f, 37.797f, 0.036f, -1.297f, -1.0f, -1.0f, -1.0f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, -1.0f},
    {237.0f, 27.0f, 36.0f, 51.588f, 73.518f, 51.569f, -1.0f, -1.0f, -1.0f},
    {254.0f, 242.0f, 0.0f, 93.699f, -15.734f, 91.942f, -1.0f, -1.0f, -1.0f},
    {106.0f, 189.0f, 71.0f, 69.408f, -46.594f, 50.487f, -1.0f, -1.0f, -1.0f},
    {0.0f, 173.0f, 239.0f, 66.610f, -13.679f, -43.172f, -1.0f, -1.0f, -1.0f},
    {0.0f, 26.0f, 83.0f, 11.711f, 16.980f, -37.176f, -1.0f, -1.0f, -1.0f},
    {238.0f, 1.0f, 141.0f, 51.974f, 81.944f, -8.407f, -1.0f, -1.0f, -1.0f},
    {174.0f, 50.0f, 58.0f, 40.549f, 50.440f, 24.849f, -1.0f, -1.0f, -1.0f},
    {209.0f, 127.0f, 41.0f, 60.816f, 26.069f, 49.442f, -1.0f, -1.0f, -1.0f},
    {7.0f, 136.0f, 165.0f, 52.253f, -19.950f, -23.996f, -1.0f, -1.0f, -1.0f},
    {188.0f, 86.0f, 149.0f, 51.286f, 48.470f, -15.058f, -1.0f, -1.0f, -1.0f},
    {200.0f, 159.0f, 139.0f, 68.707f, 12.296f, 16.213f, -1.0f, -1.0f, -1.0f},
    {183.0f, 147.0f, 125.0f, 63.684f, 10.293f, 16.764f, -1.0f, -1.0f, -1.0f},
};

int main(int argc, char *argv[])
{

    // ----------------------------------------------------------
    // Scroll down a bit (~40 lines) to find actual relevant code
    // ----------------------------------------------------------

    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    int t = parser.get<int>("t");
    int nc = parser.get<int>("nc");
    std::string filepath = parser.get<std::string>("f");

    CV_Assert(0 <= t && t <= 2);
    TYPECHART chartType = TYPECHART(t);

    std::cout << "t: " << t << " , nc: " << nc <<  ", \nf: " << filepath << std::endl;

    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }

    Mat image = cv::imread(filepath, IMREAD_COLOR);
    if (!image.data)
    {
        std::cout << "Invalid Image!" << std::endl;
        return 1;
    }

    cv::Mat imageCopy = image.clone();
    Ptr<CCheckerDetector> detector = CCheckerDetector::create();
    // Marker type to detect
    if (!detector->process(image, chartType, nc))
    {
        printf("ChartColor not detected \n");
        return 2;
    }
    // get checker
    std::vector<Ptr<mcc::CChecker>> checkers = detector->getListColorChecker();
    cout << "num of checkers" << checkers.size() << std::endl;

    
    for (Ptr<mcc::CChecker> checker : checkers)
    {
        // current checker
        Ptr<CCheckerDraw> cdraw = CCheckerDraw::create(checker);
        cdraw->draw(image);
        cv::Mat chartsRGB = checker->getChartsRGB();

        cv::Mat src = chartsRGB.col(1).clone().reshape(3, 18);
        src /= 255.0;
        cv::Mat ref_ = cv::Mat(18, 9, CV_32FC1, CChartVinylColors);
        ref_.convertTo(ref_, CV_64F);
        Mat ref = ref_.colRange(3, 6).clone().reshape(3, 18);

        Color color(ref, Lab_D50_2);
        Color color2 = color.to(sRGB);
        //cout << "color2:" << color2.colors << endl;
        std::vector<double> saturated_threshold = { 0, 0.98 };

        cv::Mat weight_list;
        ColorCorrectionModel p1(src, color, sRGB, CCM_3x3, CIE2000, COLORPOLYFIT, 2.2, 3, saturated_threshold, weight_list, 0, LEAST_SQUARE, 5000, 1e-4);
        //ColorCorrectionModel p1(src / 255, color, sRGB, CCM_3x3, CIE2000, GAMMA, 2.2, 3, saturated_threshold, weight_list, 0, LEAST_SQUARE, 5000, 1e-4);
        cv::Mat calibratedImage = p1.inferImage(filepath);
        cv::imwrite("./calibrated_image.png", calibratedImage);
        //cout << p1.loss << endl;

        //cout << "ref: " << ref << endl;
        cout << "src: " << src << endl;



        //cout << "ref:" << ref << endl;

    }
    //cv::imwrite("./original.png", image)
    cv::imwrite("./chart_color_detected.png", image);
    return 0;
}
