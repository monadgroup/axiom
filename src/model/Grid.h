#pragma once

#include <QtCore/QPoint>
#include <unordered_map>
#include <memory>
#include <QtCore/QSize>
#include <cmath>
#include <set>
#include <unordered_set>
#include <iostream>

namespace std {
    template<>
    struct hash<QPoint> {
        std::size_t operator()(const QPoint &p) const {
            return std::hash<int>()(p.x()) ^ (std::hash<int>()(p.y()) >> 1);
        }
    };
}

namespace AxiomModel {

    template<class T>
    class Grid {
    public:
        const size_t quadWidth;
        const size_t quadHeight;

        Grid(size_t quadWidth, size_t quadHeight) : quadWidth(quadWidth), quadHeight(quadHeight) {

        }

        bool isRectAvailable(QPoint pos, QSize size, const T *ignore = nullptr) const {
            for (auto dx = 0; dx < size.width(); dx++) {
                for (auto dy = 0; dy < size.height(); dy++) {
                    auto checkP = pos + QPoint(dx, dy);
                    auto found = getCell(checkP);
                    if (found && found != ignore) {
                        return false;
                    }
                }
            }
            return true;
        }

        QPoint findNearestAvailable(QPoint pos, QSize size, const T *ignore = nullptr) const {
            // breadth-first search to find nearest free area
            std::set<SortedPos> positionQueue;
            std::unordered_set<QPoint> visitedQueue;

            positionQueue.insert(SortedPos(pos, pos));
            visitedQueue.insert(pos);
            while (!positionQueue.empty()) {
                auto oldQueue = std::move(positionQueue);
                positionQueue = std::set<SortedPos>();

                for (const auto &p : oldQueue) {
                    if (isRectAvailable(p.checkPos, size, ignore)) return p.checkPos;

                    QPoint offsets[] = {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)};
                    for (const auto &offset : offsets) {
                        auto newP = p.checkPos + offset;
                        if (visitedQueue.find(newP) == visitedQueue.end()) {
                            positionQueue.insert(SortedPos(newP, pos));
                            visitedQueue.insert(newP);
                        }
                    }
                }
            }

            // oh no! this should never happen :'(
            return pos;
        }

        T *getCell(QPoint pos) const {
            auto cell = cells.find(pos);
            if (cell == cells.end()) return nullptr;
            return cell->second;
        }

        void setCell(QPoint pos, T *item) {
            if (item == nullptr) cells.erase(pos);
            else cells.emplace(pos, item);
        }

        void setRect(QPoint pos, QSize size, T *item) {
            for (auto dx = 0; dx < size.width(); dx++) {
                for (auto dy = 0; dy < size.height(); dy++) {
                    auto checkP = pos + QPoint(dx, dy);
                    setCell(checkP, item);
                }
            }
        }

        void moveRect(QPoint oldPos, QSize oldSize, QPoint newPos, QSize newSize, T *item) {
            setRect(oldPos, oldSize, nullptr);
            setRect(newPos, newSize, item);
        }

    private:
        class SortedPos {
        public:
            QPoint checkPos;
            QPoint basePos;

            SortedPos(QPoint checkPos, QPoint basePos) : checkPos(checkPos), basePos(basePos) {}

            float dist() const {
                return std::sqrt((float) checkPos.x() * checkPos.x() + checkPos.y() * checkPos.y());
            }

            bool operator<(const SortedPos &other) const {
                return dist() < other.dist();
            }
        };

        std::unordered_map<QPoint, T*> cells;
    };

}
