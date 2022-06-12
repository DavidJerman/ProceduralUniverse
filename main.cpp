#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"

constexpr uint32_t starColorsARGB[8] = {
        0xFFFFFFFF, 0xFFD9FFFF, 0xFFA3FFFF, 0xFFFFC8C8,
        0xFFFFCB9D, 0xFF9F9FFF, 0xFF415EFF, 0xFF28199D
};

constexpr uint32_t planetColorsARGB[8] = {
        0xFF042d63, 0xFFf87936, 0xFFe1eff0, 0xFF13ee3f,
        0xFFB9dec0, 0xFFDeb9dd, 0xFFDddeb9, 0xFFE87896
};

/**
 * A planet with many properties
 */
class Planet {
public:
    olc::Pixel color{0xffBb9910};
    double distance{0};
    double diameter{0};
    bool flora{false};
    std::vector<std::string> minerals{};
    bool water{false};
    std::vector<std::string> gasses{};
    double temperature{0};
    double population{0};
    bool ring = false;
    std::vector<double> moons;
};

/**
 * Star system, that might contain planets
 */
class StarSystem {
public:
    bool starExists = false;
    double starDiameter = 0.0f;
    olc::Pixel starColor = olc::WHITE;
    std::vector<Planet> planets;
    std::vector<std::string> MINERALS{"Iron", "Aluminum", "Calcium", "Potassium", "Zinc", "Sodium", "Uranium"};
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

            p.color = planetColorsARGB[rndInt(0, 8)];

            p.distance = dDistanceFromStar;

            dDistanceFromStar += rndDouble(20.0f, 200.0f);

            p.diameter = rndDouble(5.0f, 20.0f);

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

            p.temperature = rndInt(-273, 300);

            // Have a possibility of fauna only if there is water and right temperature
            if (p.water && p.temperature > 0 && p.temperature < 50)
                p.flora = (rndInt(0, 2) == 1);

            p.population = std::max(rndInt(-10000000, 9000000), 0);

            p.ring = rndInt(0, 10) == 1;

            int nMoons = std::max(rndInt(-5, 5), 0);
            for (int n = 0; n < nMoons; n++) {
                p.moons.push_back(rndDouble(1.0, 5.0));
            }
            planets.push_back(p);
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

/**
 * A galaxy containing many star systems
 */
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

    olc::vf2d galaxyOffset = {0, 0};
    bool starSelected{false};
    olc::vi2d selectedStarPosition{0, 0};

    bool OnUserCreate() override {
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
        if (GetKey(olc::W).bHeld) galaxyOffset.y -= 50.0f * fElapsedTime;
        if (GetKey(olc::S).bHeld) galaxyOffset.y += 50.0f * fElapsedTime;
        if (GetKey(olc::A).bHeld) galaxyOffset.x -= 50.0f * fElapsedTime;
        if (GetKey(olc::D).bHeld) galaxyOffset.x += 50.0f * fElapsedTime;

        Clear(olc::BLACK);

        int nSectorX = ScreenWidth() / SECTOR_SIZE;
        int nSectorY = ScreenHeight() / SECTOR_SIZE;

        olc::vi2d mouse = {GetMouseX() / SECTOR_SIZE, GetMouseY() / 16};
        olc::vi2d galaxyMouse = mouse + galaxyOffset;

        olc::vi2d screenSector = {0, 0};

        // Draw each sector
        for (screenSector.x = 0; screenSector.x < nSectorX; screenSector.x++)
            for (screenSector.y = 0; screenSector.y < nSectorY; screenSector.y++) {
                StarSystem star(screenSector.x + (uint32_t) galaxyOffset.x,
                                screenSector.y + (uint32_t) galaxyOffset.y);

                // If the star exists, draw it
                if (star.starExists) {
                    FillCircle(screenSector.x * SECTOR_SIZE + SECTOR_SIZE / 2,
                               screenSector.y * SECTOR_SIZE + SECTOR_SIZE / 2,
                               (int) star.starDiameter / (SECTOR_SIZE / 2), star.starColor);

                    if (mouse.x == screenSector.x && mouse.y == screenSector.y) {
                        DrawCircle(screenSector.x * SECTOR_SIZE + SECTOR_SIZE / 2,
                                   screenSector.y * SECTOR_SIZE + SECTOR_SIZE / 2,
                                   12, olc::BLUE);
                    }
                }
            }

        // If the planet is selected, draw the planets
        if (GetMouse(0).bPressed) {
            StarSystem star(galaxyMouse.x, galaxyMouse.y);

            if (star.starExists) {
                starSelected = true;
                selectedStarPosition = galaxyMouse;
            } else
                starSelected = false;
        }

        if (starSelected) {
            StarSystem star(selectedStarPosition.x, selectedStarPosition.y, true);

            // Windows
            FillRect(PLANETS_WINDOW_X, PLANETS_WINDOW_Y, PLANETS_WINDOW_W, PLANETS_WINDOW_H, olc::DARK_BLUE);
            DrawRect(PLANETS_WINDOW_X, PLANETS_WINDOW_Y, PLANETS_WINDOW_W, PLANETS_WINDOW_H, olc::WHITE);

            const double RATIO = 1.375;

            // Star
            olc::vi2d bodyPosition = {14, 356};
            bodyPosition.x += (int) (star.starDiameter * RATIO);
            FillCircle(bodyPosition, (int) (star.starDiameter * RATIO), star.starColor);
            bodyPosition.x += (int) (star.starDiameter * RATIO) + 8;

            // Draw planets
            for (const auto &planet: star.planets) {
                if (bodyPosition.x + planet.diameter >= PLANETS_WINDOW_W - 20) break;

                bodyPosition.x += (int) planet.diameter;
                FillCircle(bodyPosition, (int) (planet.diameter * 1.0), planet.color);

                olc::vi2d moonPosition = bodyPosition;
                moonPosition.y += (int) planet.diameter + 10;

                // Draw moons
                for (const auto &moon: planet.moons) {
                    if (moonPosition.y >= 450) break;
                    moonPosition.y += (int) moon;
                    FillCircle(moonPosition, (int) (moon * 1.0), olc::GREY);
                    moonPosition.y += (int) moon + 10;
                }

                bodyPosition.x += (int) planet.diameter + 8;
            }

            // Display information for a selected planet
            if (GetKey(olc::K1).bHeld && !star.planets.empty())
                printPlanetInfo(star.planets[0], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K2).bHeld && star.planets.size() > 1)
                printPlanetInfo(star.planets[1], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K3).bHeld && star.planets.size() > 2)
                printPlanetInfo(star.planets[2], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K4).bHeld && star.planets.size() > 3)
                printPlanetInfo(star.planets[3], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K5).bHeld && star.planets.size() > 4)
                printPlanetInfo(star.planets[4], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K6).bHeld && star.planets.size() > 5)
                printPlanetInfo(star.planets[5], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K7).bHeld && star.planets.size() > 6)
                printPlanetInfo(star.planets[6], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K8).bHeld && star.planets.size() > 7)
                printPlanetInfo(star.planets[7], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
            if (GetKey(olc::K9).bHeld && star.planets.size() > 8)
                printPlanetInfo(star.planets[8], PLANETS_WINDOW_X + 10, PLANETS_WINDOW_Y - 140);
        }

        return true;
    }

    void printPlanetInfo(const Planet &planet, const int offsetX, int offsetY) {
        FillRect(PLANETS_WINDOW_X, offsetY - 10 + 40, PLANETS_WINDOW_W, 100, olc::DARK_BLUE);
        DrawRect(PLANETS_WINDOW_X, offsetY - 10 + 40, PLANETS_WINDOW_W, 100, olc::WHITE);

        std::stringstream stream;

        stream << "Distance from sun: " << planet.distance << " u" << "\nDiameter: " << planet.diameter << " u"
               << "\nFlora: " << (planet.flora ? "Yes" : "No") << "\nMinerals: ";
        if (planet.minerals.empty()) stream << "None";
        else for (const auto &mineral: planet.minerals) stream << mineral << " ";
        stream << "\nWater: " << (planet.water ? "Yes" : "No") << "\nGasses: ";
        if (planet.gasses.empty()) stream << "None";
        else for (const auto &gas: planet.gasses) stream << gas << " ";
        stream << "\nTemperature: " << planet.temperature << " C"
               << "\nPopulation: " << planet.population
               << "\nRing: " << (planet.ring ? "Yes" : "No");

        DrawString({offsetX, offsetY + 40}, stream.str());
    }
};

int main() {
    if (Galaxy demo; demo.Construct(512, 512, 2, 2))
        demo.Start();

    return 0;
}
