//
// Created by Emin Tunc Kirimlioglu on 5/10/24.
//

#ifndef BABYGIS_HPP
#define BABYGIS_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include "misc/Extension.hpp"
#include "storage/Storage.hpp"
namespace ECE141{

    class BBox : public Storable{
    public:
        BBox(double minx, double miny, double maxx, double maxy, int srid)
            : minx(minx), miny(miny), maxx(maxx), maxy(maxy), srid(srid) {}

        virtual StatusResult encode(std::ostream &anOutput) const override {
            anOutput.write(reinterpret_cast<const char*>(&minx), sizeof(double));
            anOutput.write(reinterpret_cast<const char*>(&miny), sizeof(double));
            anOutput.write(reinterpret_cast<const char*>(&maxx), sizeof(double));
            anOutput.write(reinterpret_cast<const char*>(&maxy), sizeof(double));
            anOutput.write(reinterpret_cast<const char*>(&srid), sizeof(int));

            if (anOutput.good()) {
                return {Errors::noError};
            } else {
                return {Errors::invalidArguments};
            }
        }

        virtual StatusResult decode(std::istream &anInput) override {
            anInput.read(reinterpret_cast<char*>(&minx), sizeof(double));
            anInput.read(reinterpret_cast<char*>(&miny), sizeof(double));
            anInput.read(reinterpret_cast<char*>(&maxx), sizeof(double));
            anInput.read(reinterpret_cast<char*>(&maxy), sizeof(double));
            anInput.read(reinterpret_cast<char*>(&srid), sizeof(int));

            if (anInput.good()) {
                return {Errors::noError};
            } else {
                return {Errors::invalidArguments};
            }
        }
        virtual bool          initHeader(Block &aBlock) const override{
            return true;
        }

        [[nodiscard]] bool contains(const BBox& other) const {
            return minx <= other.minx && miny <= other.miny && maxx >= other.maxx && maxy >= other.maxy && srid == other.srid;
        }

        [[nodiscard]] bool comparable(const BBox& other) const {
            return srid == other.srid;
        }

        [[nodiscard]] BBox warp(int target_srid) const {
            // Fake transformation logic
            double scale_factor = static_cast<double>(target_srid) / srid;
            double new_minx = minx * scale_factor;
            double new_miny = miny * scale_factor;
            double new_maxx = maxx * scale_factor;
            double new_maxy = maxy * scale_factor;
            return {new_minx, new_miny, new_maxx, new_maxy, target_srid};
        }

    private:
        double minx, miny, maxx, maxy;
        int srid;
    };
}

#endif //BABYGIS_HPP
