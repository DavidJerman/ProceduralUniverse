#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"

constexpr uint32_t starColorsARGB[8] = {
        0xFFFFFFFF, 0xFFD9FFFF, 0XFFA3FFFF, 0xFFFFC8C8,
        0xFFFFCB9D, 0xFF9F9FFF, 0xFF415EFF, 0xFF28199D
};

class Planet {
public:
    double distance{0};
    double diameter{0};
    bool flora{false};
    std::vector<std::string> minerals{};
    bool water{false};
    std::vector<std::string> gasses{};
    double temperature{0};
    double population{0};
    bool ring = false;
    std::vector<double> vMoons;
};

class StarSystem {
public:
    bool starExists = false;
    double starDiameter = 0.0f;
    olc::Pixel starColor = olc::WHITE;
    std::vector<Planet> vPlanets;
    std::vector<std::string> MINERALS{"Iron", "Aluminum", "Calcium", "Potassium", "Zinc", "Sodium", "Uranium",
                                      "Magnesium", "Phosphorus", "Iodine"};
    std::vector<std::string> GASSES{"He", "O2", "N2", "H2", "CH4", "CO2"};

    StarSystem(uint32_t x, uint32_t y, bool GenerateFullSystem = false)
            : lehmerState((x & 0xFFFF) << 16 | (y & 0xFFFF)) {

        starExists = rndInt(0, 20) == 1;
        if (!starExists) return;

        starDiameter = rndDouble(10., 40.);
        starColor.n = starColorsARGB[rndInt(0, 8)];

        if (!GenerateFullSystem) return;

        double dDistanceFromStar = rndDouble(60.0f, 200.0f);
        int nPlanets = rndInt(0, 10);

        // Generate planet properties
        for (int i = 0; i < nPlanets; i++) {
            Planet p;
            p.distance = dDistanceFromStar;
            dDistanceFromStar += rndDouble(20.0f, 200.0f);
            p.diameter = rndDouble(5.0f, 20.0f);
            p.flora = (rndInt(0, 100) == 1);

            // Minerals
            auto numOfMinerals = rndInt(0, (int) MINERALS.size() - 1);
            while (numOfMinerals > 0) {
                auto pick = rndInt(0, (int) MINERALS.size());
                p.minerals.push_back(MINERALS[pick]);
                const std::string &_mineral = MINERALS[pick];
                MINERALS.erase(
                        std::remove_if(MINERALS.begin(), MINERALS.end(), [&_mineral](const std::string &mineral) {
                            return _mineral == mineral;
                        }), MINERALS.end());
                numOfMinerals--;
            }

            p.water = (rndInt(0, 10) == 1);

            // Gasses
            auto numOfGasses = rndInt(0, (int) GASSES.size() - 1);
            while (numOfGasses > 0) {
                auto pick = rndInt(0, (int) GASSES.size());
                p.gasses.push_back(GASSES[pick]);
                const std::string &_gas = GASSES[pick];
                GASSES.erase(
                        std::remove_if(GASSES.begin(), GASSES.end(), [&_gas](const std::string &gas) {
                            return _gas == gas;
                        }), GASSES.end());
                numOfGasses--;
            }

            p.temperature = rndInt(-200, 300);

            p.population = std::max(rndInt(-10000000, 9000000), 0);

            p.ring = rndInt(0, 10) == 1;

            int nMoons = std::max(rndInt(-5, 5), 0);
            for (int n = 0; n < nMoons; n++) {
                p.vMoons.push_back(rndDouble(1.0, 5.0));
            }
            vPlanets.push_back(p);
        }
    }

private:
    // A pseudo random number generator
    uint32_t lehmerState = 0;

    uint32_t Lehmer32() {
        lehmerState += 0xe120fc15;
        uint64_t tmp;
        tmp = (uint64_t) lehmerState * 0x4a39b70d;
        uint32_t m1 = (tmp >> 32) ^ tmp;
        tmp = (uint64_t) m1 * 0x12fad5c9;
        uint32_t m2 = (tmp >> 32) ^ tmp;
        return m2;
    }

    int rndInt(int min, int max) {
        return (int) (Lehmer32() % (max - min)) + min;
    }

    double rndDouble(double min, double max) {
        return ((double) Lehmer32() / (double) (0x7FFFFFFF)) * (max - min) + min;
    }
};

class Galaxy : public olc::PixelGameEngine {
    static const int SECTOR_SIZE = 16;

    static const int PLANETS_WINDOW_X = 8;
    static const int PLANETS_WINDOW_Y = 240;
    static const int PLANETS_WINDOW_W = 496;
    static const int PLANETS_WINDOW_H = 232;

public:
    Galaxy() {
        sAppName = "Galaxy View";
    }

    olc::vf2d vGalaxyOffset = {0, 0};
    bool bStarSelected{false};
    olc::vi2d vStarSelected{0, 0};
    bool bPlanetSelect{false};

    bool OnUserCreate() override {
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
        if (GetKey(olc::W).bHeld) vGalaxyOffset.y -= 50.0f * fElapsedTime;
        if (GetKey(olc::S).bHeld) vGalaxyOffset.y += 50.0f * fElapsedTime;
        if (GetKey(olc::A).bHeld) vGalaxyOffset.x -= 50.0f * fElapsedTime;
        if (GetKey(olc::D).bHeld) vGalaxyOffset.x += 50.0f * fElapsedTime;

        Clear(olc::BLACK);

        int nSectorX = ScreenWidth() / SECTOR_SIZE;
        int nSectorY = ScreenHeight() / SECTOR_SIZE;

        olc::vi2d mouse = {GetMouseX() / SECTOR_SIZE, GetMouseY() / 16};
        olc::vi2d galaxy_mouse = mouse + vGalaxyOffset;

        olc::vi2d screen_sector = {0, 0};

        // Draw each sector
        for (screen_sector.x = 0; screen_sector.x < nSectorX; screen_sector.x++)
            for (screen_sector.y = 0; screen_sector.y < nSectorY; screen_sector.y++) {
                StarSystem star(screen_sector.x + (uint32_t) vGalaxyOffset.x,
                                 screen_sector.y + (uint32_t) vGalaxyOffset.y);

                // If the star exists, draw it
                if (star.starExists) {
                    FillCircle(screen_sector.x * SECTOR_SIZE + SECTOR_SIZE / 2,
                               screen_sector.y * SECTOR_SIZE + SECTOR_SIZE / 2,
                               (int) star.starDiameter / (SECTOR_SIZE / 2), star.starColor);

                    if (mouse.x == screen_sector.x && mouse.y == screen_sector.y) {
                        DrawCircle(screen_sector.x * SECTOR_SIZE + SECTOR_SIZE / 2,
                                   screen_sector.y * SECTOR_SIZE + SECTOR_SIZE / 2,
                                   12, olc::BLUE);
                    }
                }
            }

        // If the planet is selected, draw the planets
        if (GetMouse(0).bPressed) {
            StarSystem star(galaxy_mouse.x, galaxy_mouse.y);

            if (star.starExists) {
                bStarSelected = true;
                vStarSelected = galaxy_mouse;
            } else
                bStarSelected = false;
        }

        if (bStarSelected) {
            StarSystem star(vStarSelected.x, vStarSelected.y, true);

            // Windows
            FillRect(PLANETS_WINDOW_X, PLANETS_WINDOW_Y, PLANETS_WINDOW_W, PLANETS_WINDOW_H, olc::DARK_BLUE);
            DrawRect(PLANETS_WINDOW_X, PLANETS_WINDOW_Y, PLANETS_WINDOW_W, PLANETS_WINDOW_H, olc::WHITE);

            const double RATIO = 1.375;

            // Star
            olc::vi2d vBody = {14, 356};
            vBody.x += star.starDiameter * RATIO;
            FillCircle(vBody, (int) (star.starDiameter * RATIO), star.starColor);
            vBody.x += (int) (star.starDiameter * RATIO) + 8;

            // Draw planets
            for (const auto &planet: star.vPlanets) {
                if (vBody.x + planet.diameter >= PLANETS_WINDOW_W - 20) break;

                vBody.x += planet.diameter;
                FillCircle(vBody, (int) (planet.diameter * 1.0), olc::RED);

                olc::vi2d vMoon = vBody;
                vMoon.y += planet.diameter + 10;

                // Draw moons
                for (const auto &moon: planet.vMoons) {
                    if (vMoon.y >= 450) break;
                    vMoon.y += moon;
                    FillCircle(vMoon, (int) (moon * 1.0), olc::GREY);
                    vMoon.y += moon + 10;
                }

                vBody.x += planet.diameter + 8;
            }

            // Display information for a selected planet
            if (GetKey(olc::K1).bHeld)
                if (star.vPlanets.size() > 0)
                    printPlanetInfo(star.vPlanets[0], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K2).bHeld)
                if (star.vPlanets.size() > 1)
                    printPlanetInfo(star.vPlanets[1], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K3).bHeld)
                if (star.vPlanets.size() > 2)
                    printPlanetInfo(star.vPlanets[2], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K4).bHeld)
                if (star.vPlanets.size() > 3)
                    printPlanetInfo(star.vPlanets[3], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K5).bHeld)
                if (star.vPlanets.size() > 4)
                    printPlanetInfo(star.vPlanets[4], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K6).bHeld)
                if (star.vPlanets.size() > 5)
                    printPlanetInfo(star.vPlanets[5], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K7).bHeld)
                if (star.vPlanets.size() > 6)
                    printPlanetInfo(star.vPlanets[6], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K8).bHeld)
                if (star.vPlanets.size() > 7)
                    printPlanetInfo(star.vPlanets[7], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K9).bHeld)
                if (star.vPlanets.size() > 8)
                    printPlanetInfo(star.vPlanets[8], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
        }

        return true;
    }

    void printPlanetInfo(const Planet &planet, const int offsetX, int offsetY) {
        FillRect(PLANETS_WINDOW_X, PLANETS_WINDOW_Y - 150, PLANETS_WINDOW_W, 140, olc::DARK_BLUE);
        DrawRect(PLANETS_WINDOW_X, PLANETS_WINDOW_Y - 150, PLANETS_WINDOW_W, 140, olc::WHITE);

        std::stringstream stream;

        stream << "Distance: " << planet.distance << "\nDiameter: " << planet.diameter
        << "\nFlora: " << (planet.flora ? "Yes" : "No") << "\nMinerals: ";
        for (const auto &mineral: planet.minerals) stream << mineral << " ";
        stream << "\nWater: " << (planet.water ? "Yes" : "No") << "\nGasses: ";
        for (const auto &gas: planet.gasses) stream << gas << " ";
        stream << "\nTemperature: " << planet.temperature << " K"
        << "\nPopulation: " << planet.population
        << "\nRing: " << (planet.ring ? "Yes" : "No");

        DrawString({offsetX, offsetY}, stream.str());
    }
};

int main() {
    if (Galaxy demo; demo.Construct(512, 512, 2, 2))
        demo.Start();

    return 0;
}
