#include "imgui.h" // necessary for ImGui::*, imgui-SFML.h doesn't include imgui.h

#include "imgui-SFML.h" // for ImGui::SFML::* functions and SFML-specific overloads

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <opencv4/opencv2/opencv.hpp>

struct WebcapPanal
{
    size_t capture_id;
    cv::VideoCapture capture;
    size_t resolution_x;
    size_t resolution_y;

    // HACK: rendering data. Need somewhere to store it.
    // No time to properlly engineer this
    std::vector<std::vector<float>> histogram = std::vector<std::vector<float>>(3);
    bool pulse = false;
    cv::Mat image;

};

sf::RenderWindow window;
std::deque<std::shared_ptr<WebcapPanal>> panals;

void render_panal(WebcapPanal& panal)
{
    cv::Mat& image = panal.image;
    if(!panal.pulse) {
        panal.capture.read(image);
    }
    int width = image.cols;
    int height = image.rows;

    assert(width == panal.resolution_x);
    assert(height == panal.resolution_y);

    ImGui::Begin(("Camera " + std::to_string(panal.capture_id)).c_str());
    ImGui::LabelText("Image size", "%dx%d", width, height);

    // Don't have time to figure out how to get opencv image shown on ImGui. Use imshow
    // FIXME: This should have worked. Didn't
    // sf::Image dispimg;
    // dispimg.create(width, height, image.data);
    // sf::Texture dispimg_texture;
    // ImGui::Image(dispimg_texture);
    // sf::Sprite sprite(dispimg_texture);
    // window.draw(sprite);
    // Wasted 30 minutes on this

    // Code yanked from OpenCV's website. Out of time at this point
    std::vector<cv::Mat> bgr_planes;
    cv::split(image, bgr_planes);
    int i=0;
    for(auto plane : bgr_planes) {
        auto& histogram = panal.histogram[i];
        histogram = std::vector<float>(256, 0.f);
        for(auto it=image.begin<uint8_t>(); it!=image.end<uint8_t>(); it++) {
            histogram[*it] ++;
        }

        ImGui::PlotHistogram(("Histo " + std::to_string(i)).c_str(), histogram.data(), histogram.size());
        i++;
    }

    bool close_window = ImGui::Button("Close this window");
    if(close_window) {
        for(auto it = panals.begin(); it != panals.end(); it++) {
            // HACK: Bad design
            if(it->get() == &panal) {
                panals.erase(it);
                break;
            }
        }
    }
    ImGui::Checkbox("Pulse", &panal.pulse);
    
    ImGui::End();

    // HACK: Show OpenCV image
    if(!close_window)
        cv::imshow("camera "+std::to_string(panal.capture_id), image);
    else
        cv::destroyWindow("camera "+std::to_string(panal.capture_id));
}

int main() {
    window.create(sf::VideoMode(640, 480), "Camera APP");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        static int new_cam_id = 0;
        // 720p by default
        static int resolution_x = 1280;
        static int resolution_y = 720;
        bool create_cam_button_pressed = false;

        ImGui::SFML::Update(window, deltaClock.restart());


        ImGui::Begin("Control Window");
            ImGui::InputInt("Camera ID", &new_cam_id);
            ImGui::InputInt("X resolution", &resolution_x);
            ImGui::InputInt("Y resolution", &resolution_y);
            create_cam_button_pressed = ImGui::Button("Create new panal!");
        ImGui::End();

        if(create_cam_button_pressed) {
            // TODO:  Write a constructor to build the object
            std::shared_ptr<WebcapPanal> panal = std::make_shared<WebcapPanal>();
            panal->capture_id = new_cam_id;
            panal->capture = cv::VideoCapture(new_cam_id);
            panal->resolution_x = resolution_x;
            panal->resolution_y = resolution_y;
            // TODO: Add configurable options
            panal->capture.set(cv::CAP_PROP_FRAME_WIDTH, resolution_x);
            panal->capture.set(cv::CAP_PROP_FRAME_HEIGHT, resolution_y);
            panals.emplace_back(std::move(panal));
        }

        // HACK: Prevent deallocation of the panal data. Otherwise when we remove panal from the list
        // the panal will deallocate then ImGUI draws it. Causing use afetr free. Again, no time to 
        // properlly engineer this
        auto panals_holder = panals;
        
        for(auto& panal : panals)
            render_panal(*panal);

        window.clear();
        ImGui::SFML::Render(window);
        window.display();

        // HACK: Because I'm displaying via OpenCV now
        cv::waitKey(1);
    }

    ImGui::SFML::Shutdown();

    return 0;
}
