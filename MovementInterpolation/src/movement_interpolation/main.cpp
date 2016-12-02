#include <game_loop/game_loop.h>
#include <factory/render_object_factory.h>
#include <rendering/renderer.h>

#include <memory>
#include <movement_interpolation/gui/movement_interpolation_gui.h>
#include <movement_interpolation/interpolation_simulation.h>
#include <factory/texture_factory.h>
#include <factory/program_factory.h>
#include <model_loader/model_loader.h>

void InitScene(ifx::GameLoop& game_loop);
void InitSimulation(ifx::GameLoop& game_loop);

std::shared_ptr<ifx::Model> CreateAxisModel();
std::shared_ptr<ifx::RenderObject> CreateAxis();

void InitScene(ifx::GameLoop& game_loop){
    game_loop.renderer()->scene()->AddRenderObject(
            ifx::RenderObjectFactory().CreateQuad());
}

void InitSimulation(ifx::GameLoop& game_loop){
    auto simulation = std::shared_ptr<InterpolationSimulation>(
            new InterpolationSimulation(
                    game_loop.renderer()->scene(),
                    game_loop.renderer(),
                    CreateAxis()));
    auto gui = std::unique_ptr<MovementInterpolationGUI>(
            new MovementInterpolationGUI(
                    game_loop.renderer()->window()->getHandle(),
                    game_loop.renderer(),
                    simulation));
    game_loop.renderer()->SetGUI(std::move(gui));
    game_loop.AddSimulation(simulation);
}

std::shared_ptr<ifx::RenderObject> CreateAxis(){
    std::shared_ptr<Program> program = ifx::ProgramFactory().LoadMainProgram();
    std::string path
            = ifx::Resources::GetInstance().GetResourcePath(
                    "axis-obj/axis.obj", ifx::ResourceType::MODEL);
    auto model = ifx::ModelLoader(path).loadModel();

    auto render_object
            = std::shared_ptr<ifx::RenderObject>(
                    new ifx::RenderObject(ObjectID(0),
                                          model));
    render_object->addProgram(program);
    render_object->scale(0.4);
    return render_object;
}

int main() {
    ifx::GameLoop game_loop(
            std::move(ifx::RenderObjectFactory().CreateRenderer()));

    InitScene(game_loop);
    InitSimulation(game_loop);

    game_loop.Start();
}

