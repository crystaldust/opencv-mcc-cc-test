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

int main(int argc, char *argv[])
{

    // ----------------------------------------------------------
    // Scroll down a bit (~40 lines) to find actual relevant code
    // ----------------------------------------------------------

    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    int t = parser.get<int>("t");
    int nc = parser.get<int>("nc");
    string filepath = parser.get<string>("f");

    CV_Assert(0 <= t && t <= 2);
    TYPECHART chartType = TYPECHART(t);

    cout << "t: " << t << " , nc: " << nc <<  ", \nf: " << filepath << endl;

    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }

    Mat image = cv::imread(filepath, IMREAD_COLOR);
    if (!image.data)
    {
        cout << "Invalid Image!" << endl;
        return 1;
    }

    Mat imageCopy = image.clone();
    Ptr<CCheckerDetector> detector = CCheckerDetector::create();
    // Marker type to detect
    if (!detector->process(image, chartType, nc))
    {
        printf("ChartColor not detected \n");
        return 2;
    }
    // get checker
    vector<Ptr<mcc::CChecker>> checkers = detector->getListColorChecker();
    cout << "num of checkers" << checkers.size() << endl;

    
    for (Ptr<mcc::CChecker> checker : checkers)
    {
        Ptr<CCheckerDraw> cdraw = CCheckerDraw::create(checker);
        cdraw->draw(image);
        Mat chartsRGB = checker->getChartsRGB();

        Mat src = chartsRGB.col(1).clone().reshape(3, 18);
        src /= 255.0;


        Mat weight_list;
        ColorCorrectionModel model1(src, Vinyl_D50_2);
        
        // More models with different parameters, try it & check the document for details.
        // ColorCorrectionModel model2(src, Vinyl_D50_2, AdobeRGB, CCM_4x3, CIE2000, GAMMA, 2.2, 3);
        // ColorCorrectionModel model3(src, Vinyl_D50_2, WideGamutRGB, CCM_4x3, CIE2000, GRAYPOLYFIT, 2.2, 3);
        // ColorCorrectionModel model4(src, Vinyl_D50_2, ProPhotoRGB, CCM_4x3, RGBL, GRAYLOGPOLYFIT, 2.2, 3);
        // ColorCorrectionModel model5(src, Vinyl_D50_2, DCI_P3_RGB, CCM_3x3, RGB, IDENTITY_, 2.2, 3);
        // ColorCorrectionModel model6(src, Vinyl_D50_2, AppleRGB, CCM_3x3, CIE2000, COLORPOLYFIT, 2.2, 2,{ 0, 0.98 },Mat(),2);
        // ColorCorrectionModel model7(src, Vinyl_D50_2, REC_2020_RGB, CCM_3x3,  CIE94_GRAPHIC_ARTS, COLORLOGPOLYFIT, 2.2, 3);


        Mat calibratedImage = model1.inferImage(filepath);

        // Save the calibrated image to {FILE_NAME}.calibrated.{FILE_EXT}
        string filename = filepath.substr(filepath.find_last_of('/')+1);
        int dotIndex = filename.find_last_of('.');
        string baseName = filename.substr(0, dotIndex);
        string ext = filename.substr(dotIndex+1, filename.length()-dotIndex);
        string calibratedFilePath = baseName + ".calibrated." + ext;

        cv::imwrite(calibratedFilePath, calibratedImage);
    }
    return 0;
}
