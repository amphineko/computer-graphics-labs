#include "imgui.h"

#include "lib/cameras/camera_tp.h"
#include "lib/program.h"
#include "lib/skybox.h"

#define MAX_BLUR_LAYERS 16

class LensProgram : public Program {
public:
    LensProgram() { SetLightCount(1); }

    bool Initialize(const std::string &window_title) { return Initialize(window_title, true); }

private:
    ShaderProgram *phong_, *blur_, *len_, *tex_, *depth_pick_;

    Scene *obj_;

    GLuint null_vao_ = 0;
    GLuint fbo_ = 0, color_rto_ = 0, depth_rto_ = 0;
    GLuint blur_fbo_ = 0, blur_color_rto_ = 0, blur_depth_rbo_ = 0;

    GLuint depth_pick_tfo_ = 0, depth_pick_query_ = 0;
    GLuint depth_pick_vao_ = 0, depth_pick_vbo_out_ = 0;
    GLfloat picked_depth_[4] = {2.718281828459045f};
    bool enable_depth_pick_ = true;

    float depth_focus_ = 1.0f, depth_focus_max_ = 50.0f;

    float depth_scale_max_ = 15.0f, depth_scale_min_ = 5.0f;
    bool draw_depth_texture_ = false;

    GLint draw_blur_level_ = 0;
    bool draw_blur_ = false;

    glm::mat4
    ConfigureShaders(glm::vec3 camera_position, uint width, uint height, float fov, glm::mat4 view_matrix) override {
        auto projectionMatrix = Program::ConfigureShaders(camera_position, width, height, fov, view_matrix);
        auto inverseProjectionMatrix = glm::inverse(projectionMatrix);

        len_->Use();
        len_->SetMat4("inverseProjectionMatrix", inverseProjectionMatrix);

        if (enable_depth_pick_) {
            depth_pick_->Use();
            depth_pick_->SetMat4("inverseProjectionMatrix", inverseProjectionMatrix);
        }
    }

    void Draw() override {
        // draw world to texture

        Program::DrawTo(fbo_);

        phong_->Use();
        obj_->Draw(phong_);

        glCheckError();

        // draw blurred texture to levels

        Program::DrawTo(blur_fbo_);

        blur_->Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, color_rto_);

        blur_->SetInt("colorTexture", 0);

        DrawQuad();
        glCheckError();

        // draw levels with lens shader

        Program::Draw();

        len_->Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, blur_color_rto_);
        len_->SetInt("colorTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depth_rto_);
        len_->SetInt("depthTexture", 1);

        len_->SetFloat("focusDistance", depth_focus_);
        len_->SetFloat("focusDistanceMax", depth_focus_max_);

        len_->SetInt("drawDepth", draw_depth_texture_);
        len_->SetFloat("drawDepthMax", depth_scale_max_);
        len_->SetFloat("drawDepthMin", depth_scale_min_);

        len_->SetInt("drawBlur", draw_blur_);
        len_->SetInt("drawBlurLevel", draw_blur_level_);

        len_->SetFloat("zFar", Z_FAR);
        len_->SetFloat("zNear", Z_NEAR);

        DrawQuad();
        glCheckError();

        // pick depth

        if (enable_depth_pick_) {
            PickDepth();
            depth_focus_ = picked_depth_[0];
        }
    }

    void DrawImGui() override {
        Program::DrawImGui();

        ImGui::Begin("Lens Effect");

        ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::SliderFloat("Focus Distance", &depth_focus_, 0.001f, 30.0f);
        ImGui::SliderFloat("Focus Distance Max", &depth_focus_max_, 1.0f, 30.0f);
        ImGui::TreePop();

        ImGui::TreeNodeEx("Depth Texture", ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::Checkbox("Draw", &draw_depth_texture_);
        ImGui::SliderFloat("Scale Max", &depth_scale_max_, Z_NEAR, Z_FAR);
        ImGui::SliderFloat("Scale Min", &depth_scale_min_, Z_NEAR, Z_FAR);
        ImGui::TreePop();

        ImGui::TreeNodeEx("Blurred Textures", ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::Checkbox("Draw", &draw_blur_);
        ImGui::SliderInt("Level", &draw_blur_level_, 0, 5);
        ImGui::TreePop();

        ImGui::TreeNodeEx("Misc", ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::Checkbox("Pick Depth by Cursor", &enable_depth_pick_);
        if (enable_depth_pick_) {
            auto cursor = GetCursorPosition();
            ImGui::Text("Cursor Position: %f, %f", cursor.x, cursor.y);
            ImGui::Text("Picked Depth: %f", picked_depth_[0]);
        }
        ImGui::TreePop();

        ImGui::End();
    }

    void DrawQuad() const {
        glBindVertexArray(null_vao_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glm::vec2 GetCursorPosition() const {
        double x, y;
        glfwGetCursorPos(window_, &x, &y);
        return {x / window_width_, 1 - y / window_height_};
    }

    void HandleFramebufferSizeChange(int width, int height) override {
        Program::HandleFramebufferSizeChange(width, height);
        ResizeRenderTarget();
    }

    bool Initialize(const std::string &window_title, bool env_map) override {
        if (!Program::Initialize(window_title, true)) {
            return false;
        }

        // null vao for drawing full-screen quad

        glGenVertexArrays(1, &null_vao_);

        // configure camera

        camera_->SetPosition(0.0, 0.0, 0.0);
        camera_->SetRotation(0.0f, -30.0f);

        // load shaders

        phong_ = new ShaderProgram("shaders/phong.vert", "shaders/blinn-phong.frag");
        if (!phong_->IsReady()) {
            std::cerr << "FATAL: Failed to initialize phong shader" << std::endl;
            return false;
        }

        blur_ = new ShaderProgram("shaders/lens/screen.vert", "shaders/lens/blur.geom", "shaders/lens/blur.frag");
        if (!blur_->IsReady()) {
            std::cerr << "FATAL: Failed to initialize blur shader" << std::endl;
            return false;
        }

        len_ = new ShaderProgram("shaders/lens/screen.vert", "shaders/lens/lens.frag");
        if (!len_->IsReady()) {
            std::cerr << "FATAL: Failed to initialize lens shader" << std::endl;
            return false;
        }

        tex_ = new ShaderProgram("shaders/lens/screen.vert", "shaders/lens/screen.frag");
        if (!tex_->IsReady()) {
            std::cerr << "FATAL: Failed to initialize texture debug shader" << std::endl;
            return false;
        }

        depth_pick_ = new FeedbackShaderProgram(
            {
                {"shaders/lens/depth-pick.vert", GL_VERTEX_SHADER},
                {"shaders/lens/depth-pick.geom", GL_GEOMETRY_SHADER},
                {"shaders/lens/depth-pick.frag", GL_FRAGMENT_SHADER},
            },
            {"gDepth"});
        if (!depth_pick_->IsReady()) {
            std::cerr << "FATAL: Failed to initialize picker shader" << std::endl;
            return false;
        }

        shaders_.push_back(phong_);
        shaders_.push_back(blur_);
        shaders_.push_back(len_);
        shaders_.push_back(tex_);
        shaders_.push_back(depth_pick_);

        // configure lights

        SetLightCount(3);
        SetLight(0, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, -2.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        SetLight(1, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        SetLight(2, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

        // load objects

        if (!Scene::CreateFromFile("resources/models/corridor/scene.gltf", obj_, texture_manager_)) {
            std::cerr << "FATAL: Failed to load brick wall" << std::endl;
            return false;
        }
        obj_->Initialize();
        obj_->Scale(2.0f);
        obj_->SetPosition(0.0f, 0.0f, 0.0f);

        // initialize secondary render target

        glGenTextures(1, &color_rto_);
        glBindTexture(GL_TEXTURE_2D, color_rto_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenTextures(1, &depth_rto_);
        glBindTexture(GL_TEXTURE_2D, depth_rto_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenTextures(1, &blur_color_rto_);
        glBindTexture(GL_TEXTURE_2D_ARRAY, blur_color_rto_);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenRenderbuffers(1, &blur_depth_rbo_);

        glCheckError();

        ResizeRenderTarget();

        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_rto_, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_rto_, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_rto_, 0);
        glCheckError();

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "FATAL: Failed to initialize off-screen framebuffer" << std::endl;
            return false;
        }

        glGenFramebuffers(1, &blur_fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo_);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, blur_color_rto_, 0);
        glCheckError();

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "FATAL: Failed to initialize blurring framebuffer" << std::endl;
            return false;
        }

        // initialize transform feedback for depth picking

        glGenTransformFeedbacks(1, &depth_pick_tfo_);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, depth_pick_tfo_);

        glGenBuffers(1, &depth_pick_vbo_out_);
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, depth_pick_vbo_out_);
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(picked_depth_), &picked_depth_[0], GL_DYNAMIC_READ);
        glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, depth_pick_vbo_out_, 0, sizeof(picked_depth_));

        glGenQueries(1, &depth_pick_query_);

        glGenVertexArrays(1, &depth_pick_vao_);
        glBindVertexArray(depth_pick_vao_);

        glCheckError();

        // re-apply window size

        glfwGetWindowSize(window_, &window_width_, &window_height_);
        glfwSetWindowSize(window_, window_width_, window_height_);

        return true;
    }

    void PickDepth() {
        // configure drawing

        glBindVertexArray(null_vao_);

        depth_pick_->Use();
        depth_pick_->SetVec2("cursorPosition", GetCursorPosition());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth_rto_);
        depth_pick_->SetInt("depthTexture", 0);

        glCheckError();

        // configure transform feedback

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, depth_pick_tfo_);
        if (glIsTransformFeedback(depth_pick_tfo_) != GL_TRUE) {
            std::cerr << "FATAL: Failed to bind transform feedback object" << std::endl;
            return;
        }
        glBeginTransformFeedback(GL_LINES);
        glCheckError();

        // draw

        glDrawArrays(GL_POINTS, 0, 1);
        glCheckError();

        // read back

        glEndTransformFeedback();
        glCheckError();

        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(picked_depth_), &picked_depth_[0]);
    }

    void ResizeRenderTarget() {
        glBindTexture(GL_TEXTURE_2D, color_rto_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, window_width_, window_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glCheckError();

        glBindTexture(GL_TEXTURE_2D, depth_rto_);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_DEPTH_COMPONENT32,
                     window_width_,
                     window_height_,
                     0,
                     GL_DEPTH_COMPONENT,
                     GL_FLOAT,
                     nullptr);
        glCheckError();

        glBindTexture(GL_TEXTURE_2D_ARRAY, blur_color_rto_);
        glTexImage3D(GL_TEXTURE_2D_ARRAY,
                     0,
                     GL_RGBA,
                     window_width_,
                     window_height_,
                     MAX_BLUR_LAYERS,
                     0,
                     GL_RGBA,
                     GL_FLOAT,
                     nullptr);
        glCheckError();

        glBindRenderbuffer(GL_RENDERBUFFER, blur_depth_rbo_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width_, window_height_);
        glCheckError();
    }
};

int main() {
    LensProgram program;
    if (program.Initialize("Lens")) {
        program.Run();
        return 0;
    } else {
        return -1;
    }
}
