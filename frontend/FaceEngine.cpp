#include "FaceEngine.h"
#include <QFile>
#include <QDebug>
#include <cmath>
#include <algorithm>

FaceEngine::FaceEngine(const QString &cascadePath) {
    load_cascade(cascadePath);
}

FaceEngine::~FaceEngine() = default;

bool FaceEngine::load_cascade(const QString &path) {
    m_cascade_loaded = m_cascade.load(path.toStdString());
    if (!m_cascade_loaded) {
        qWarning() << "Failed to load Haar cascade from:" << path;
    } else {
        qDebug() << "Haar cascade loaded from:" << path;
    }
    return m_cascade_loaded;
}

cv::Rect FaceEngine::detect_face(const cv::Mat &frame) {
    if (!m_cascade_loaded || frame.empty()) return {};

    cv::Mat gray;
    if (frame.channels() == 3) {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = frame.clone();
    }

    cv::equalizeHist(gray, gray);

    std::vector<cv::Rect> faces;
    m_cascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(60, 60));

    if (faces.empty()) return {};

    // Return the largest face (by area)
    auto largest = std::max_element(faces.begin(), faces.end(),
        [](const cv::Rect &a, const cv::Rect &b) {
            return a.area() < b.area();
        });

    return *largest;
}

std::vector<float> FaceEngine::extract_feature(const cv::Mat &faceRegion) {
    if (faceRegion.empty()) return {};

    cv::Mat gray;
    if (faceRegion.channels() == 3) {
        cv::cvtColor(faceRegion, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = faceRegion.clone();
    }

    // Resize to standard size
    cv::Mat resized;
    cv::resize(gray, resized, cv::Size(FACE_WIDTH, FACE_HEIGHT));

    // Equalize histogram to reduce lighting effects
    cv::equalizeHist(resized, resized);

    // Convert to float and normalize to [0, 1]
    cv::Mat floatMat;
    resized.convertTo(floatMat, CV_32F, 1.0 / 255.0);

    // Flatten to feature vector
    std::vector<float> feature;
    feature.assign(floatMat.begin<float>(), floatMat.end<float>());

    // L2 normalize
    float norm = 0.0f;
    for (float v : feature) norm += v * v;
    norm = std::sqrt(norm);
    if (norm > 1e-6f) {
        for (float &v : feature) v /= norm;
    }

    return feature;
}

void FaceEngine::draw_face_rect(cv::Mat &frame, const cv::Rect &face, const QString &label) {
    cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);

    if (!label.isEmpty()) {
        int baseline = 0;
        cv::Size textSize = cv::getTextSize(label.toStdString(),
                                             cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseline);
        cv::rectangle(frame,
                      cv::Point(face.x, face.y - textSize.height - 5),
                      cv::Point(face.x + textSize.width, face.y),
                      cv::Scalar(0, 255, 0), cv::FILLED);
        cv::putText(frame, label.toStdString(),
                    cv::Point(face.x, face.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7,
                    cv::Scalar(0, 0, 0), 2);
    }
}

std::pair<int, float> FaceEngine::match_face(const std::vector<float> &feature,
                                             const std::vector<std::vector<float>> &database,
                                             float threshold) {
    if (feature.empty() || database.empty()) {
        return {-1, std::numeric_limits<float>::max()};
    }

    int bestIdx = -1;
    float bestDist = std::numeric_limits<float>::max();

    for (size_t i = 0; i < database.size(); ++i) {
        const auto &dbFeature = database[i];
        if (dbFeature.size() != feature.size()) continue;

        float dist = 0.0f;
        for (size_t j = 0; j < feature.size(); ++j) {
            float diff = feature[j] - dbFeature[j];
            dist += diff * diff;
        }
        dist = std::sqrt(dist);

        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = static_cast<int>(i);
        }
    }

    if (bestDist > threshold) {
        return {-1, bestDist};
    }

    return {bestIdx, bestDist};
}
