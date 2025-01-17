

#include "dxmc/beams/ctdibeam.hpp"
#include "dxmc/beams/ctsequentialbeam.hpp"
#include "dxmc/transport.hpp"
#include "dxmc/world/visualization/visualizeworld.hpp"
#include "dxmc/world/world.hpp"
#include "dxmc/world/worlditems/ctdiphantom.hpp"
#include "dxmc/world/worlditems/enclosedroom.hpp"
#include "dxmc/world/worlditems/fluencescore.hpp"

#include <iostream>

constexpr int NSHELLS = 12;
using Material = dxmc::Material<NSHELLS>;
using Score = dxmc::FluenceScore;
using Room = dxmc::EnclosedRoom<NSHELLS, 2>;
using CTDI = dxmc::CTDIPhantom<NSHELLS, 2>;
using World = dxmc::World<Score, Room, CTDI>;
using Beam = dxmc::CTSequentialBeam<>;

void show(const auto& world)
{
    const std::string name = "world";

    constexpr int resy = 1024 * 1;
    constexpr int resx = (resy * 3) / 2;
    constexpr double zoom = 1.5;
    dxmc::VisualizeWorld viz(world);
    viz.setDistance(700);

    auto buffer = viz.template createBuffer<double>(resx, resy);

    auto to_string = [](int n) -> std::string {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << n;
        return ss.str();
    };

    std::string message;

    constexpr int degStep = 60;
    viz.setAzimuthalAngleDeg(60);
    for (int i = 0; i < 360; i = i + degStep) {
        std::string fn = name + "Upper" + to_string(i) + ".png";
        std::cout << std::string(message.length(), ' ') << "\r";
        message = "Generating " + fn;
        std::cout << message << std::flush << "\r";
        viz.setPolarAngleDeg(i);
        viz.suggestFOV(zoom);
        viz.generate(world, buffer);
        viz.savePNG(fn, buffer);
    }

    viz.setAzimuthalAngleDeg(90);
    for (int i = 0; i < 360; i = i + degStep) {
        std::string fn = name + "Middle" + to_string(i) + ".png";
        std::cout << std::string(message.length(), ' ') << "\r";
        message = "Generating " + fn;
        std::cout << message << std::flush << "\r";
        viz.setPolarAngleDeg(i);
        viz.suggestFOV(zoom);
        viz.generate(world, buffer);
        viz.savePNG(fn, buffer);
    }

    viz.setAzimuthalAngleDeg(110);
    for (int i = 0; i < 360; i = i + degStep) {
        std::string fn = name + "Lower" + to_string(i) + ".png";
        std::cout << std::string(message.length(), ' ') << "\r";
        message = "Generating " + fn;
        std::cout << message << std::flush << "\r";
        viz.setPolarAngleDeg(i);
        viz.suggestFOV(zoom);
        viz.generate(world, buffer);
        viz.savePNG(fn, buffer);
    }
}

int main()
{

    World world;
    world.reserveNumberOfItems(3);

    auto& room = world.template addItem<Room>();
    auto& ctdi_phantom = world.template addItem<CTDI>();
    auto& score = world.template addItem<Score>();

    room.setInnerRoomAABB({ -300, -300, -460, 300, 300, 300 });
    auto lead = Material::byZ(82).value();
    const auto lead_dens = dxmc::AtomHandler::Atom(82).standardDensity;
    room.setMaterial(lead, lead_dens);

    score.setCenter({ 0, 0, -450 });
    score.setPlaneNormal({ 0, 0, 1 });
    score.setRadius(50);
    score.setEnergyStep(1);
    world.build();

    Beam beam;
    beam.setCollimation(1);
    beam.setNumberOfSlices(1);
    beam.setNumberOfParticlesPerExposure(4E5);
    beam.setStepAngleDeg(1);
    beam.setTubeVoltage(140);
    beam.addTubeFiltrationMaterial(13, 7);
    beam.setScanFieldOfView(50);

    dxmc::Transport transport;

    auto air = Material::byNistName("Air, Dry (near sea level)").value();

    const auto ctdi = [](double kVp) -> double {
        return .000050861 * kVp * kVp + .001103392 * kVp - 0.153563729;
    };

    world.clearDoseScored();
    beam.setTubeVoltage(140);
    beam.setCTDIw(ctdi(140));
    transport.runConsole(world, beam);
    auto fluence140 = score.getFluenceSpecter();

    world.clearDoseScored();
    beam.setTubeVoltage(120);
    beam.setCTDIw(ctdi(120));
    transport.runConsole(world, beam);
    auto fluence120 = score.getFluenceSpecter();

    world.clearDoseScored();
    beam.setTubeVoltage(100);
    beam.setCTDIw(ctdi(100));
    transport.runConsole(world, beam);
    auto fluence100 = score.getFluenceSpecter();

    world.clearDoseScored();
    beam.setTubeVoltage(80);
    beam.setCTDIw(ctdi(80));
    transport.runConsole(world, beam);
    auto fluence80 = score.getFluenceSpecter();

    std::cout << "Energy, u_en_air, 140kVp, 120kVp, 100kVp, 80kVp\n";
    for (std::size_t i = 0; i < fluence140.size(); ++i) {
        if (fluence140[i].first <= 140) {
            std::cout << fluence140[i].first << ", ";
            std::cout << air.massEnergyTransferAttenuation(fluence140[i].first) << ", ";
            std::cout << fluence140[i].second << ", ";
            std::cout << fluence120[i].second << ", ";
            std::cout << fluence100[i].second << ", ";
            std::cout << fluence80[i].second;
            std::cout << '\n';
        }
    }

    return 1;
}