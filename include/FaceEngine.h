#ifndef FACEENGINE_H
#define FACEENGINE_H

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <QString>
#include <vector>
#include <memory>

class FaceEngine {
public:
    explicit FaceEngine(const QString &cascadePath = "haarcascade_frontalface_default.xml");
    ~FaceEngine();

    FaceEngine(const FaceEngine &) = delete;
    FaceEngine &operator=(const FaceEngine &) = delete;

    bool load_cascade(const QString &path);
    bool is_loaded() const { return m_cascade_loaded; }

    // Detect face in frame, returns face region or empty Rect
    cv::Rect detect_face(const cv::Mat &frame);

    // Extract feature vector from face region
    std::vector<float> extract_feature(const cv::Mat &faceRegion);

    // Draw detection rectangle on frame
    void draw_face_rect(cv::Mat &frame, const cv::Rect &face, const QString &label = {});

    // Match feature against database, returns (bestIndex, distance) or (-1, INF)
    std::pair<int, float> match_face(const std::vector<float> &feature,
                                     const std::vector<std::vector<float>> &database,
                                     float threshold = 0.6f);

    static constexpr int FACE_WIDTH = 128;
    static constexpr int FACE_HEIGHT = 128;

private:
    cv::CascadeClassifier m_cascade;
    bool m_cascade_loaded{false};
};

#endif // FACEENGINE_H
