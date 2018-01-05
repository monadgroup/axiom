#pragma once

#include <QtCore/QPoint>
#include <unordered_map>
#include <memory>
#include <QtCore/QSize>
#include <cmath>
#include <set>
#include <unordered_set>
#include <iostream>
#include <queue>

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
        QPoint minRect;
        QPoint maxRect;

        Grid(QPoint minRect = QPoint(INT_MIN, INT_MIN), QPoint maxRect = QPoint(INT_MAX, INT_MAX))
                : minRect(minRect), maxRect(maxRect) {}

        bool isInsideRect(QPoint pos) const {
            return pos.x() >= minRect.x() && pos.y() >= minRect.y() && pos.x() <= maxRect.x() && pos.y() <= maxRect.y();
        }

        bool isRectAvailable(QPoint pos, QSize size, const T *ignore = nullptr) const {
            // hot path for rect being partially outside region
            auto bp = pos + QPoint(size.width(), size.height());
            if (pos.x() < minRect.x() || pos.y() < minRect.y() || bp.x() > maxRect.x() || bp.y() > maxRect.y()) {
                return false;
            }

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
            std::priority_queue<NearestAvailablePos> positionQueue;
            std::unordered_set<QPoint> visitedQueue;

            positionQueue.push(NearestAvailablePos(pos, pos));
            visitedQueue.insert(pos);

            while (!positionQueue.empty()) {
                auto p = positionQueue.top();
                positionQueue.pop();

                if (isRectAvailable(p.checkPos, size, ignore)) return p.checkPos;

                QPoint offsets[] = {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)};
                for (const auto &offset : offsets) {
                    auto newP = p.checkPos + offset;
                    if (!isInsideRect(newP)) continue;
                    if (visitedQueue.find(newP) == visitedQueue.end()) {
                        positionQueue.push(NearestAvailablePos(newP, pos));
                        visitedQueue.insert(newP);
                    }
                }
            }

            // no available position
            return pos;
        }

        std::deque<QPoint>
        findPath(QPoint start, QPoint end, float emptyCost, float filledCost, float dirChangeCost) const {
            // A* search to find shortest path
            std::priority_queue<CostPos> visitQueue;
            std::unordered_map<QPoint, VisitedCell> visited;
            std::deque<QPoint> path;

            visitQueue.push(CostPos(start, QPoint(1, 0), 0));
            visited.emplace(start, VisitedCell(start, 0));

            while (!visitQueue.empty()) {
                auto cur = visitQueue.top();

                if (cur.pos == end) {
                    auto lastVisited = end;
                    while (lastVisited != start) {
                        path.push_front(lastVisited);
                        lastVisited = visited.find(lastVisited)->second.from;
                    }
                    path.push_front(start);

                    break;
                }

                visitQueue.pop();

                QPoint dirs[] = {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)};
                for (const auto &dir : dirs) {
                    auto newP = cur.pos + dir;
                    if (!isInsideRect(newP)) continue;
                    auto newCost = (getCell(newP) ? filledCost : emptyCost) + (dir == cur.dir ? 0 : dirChangeCost);
                    auto find = visited.find(newP);
                    if (find == visited.end() || newCost < find->second.cost) {
                        visited.emplace(newP, VisitedCell(cur.pos, newCost));

                        // manhattan distance heuristic
                        auto priority = newCost + std::abs(newP.x() - end.x()) + std::abs(newP.y() - end.y());
                        visitQueue.push(CostPos(newP, dir, priority));
                    }
                }
            }

            return path;
        }

        T *getCell(QPoint pos) const {
            if (!isInsideRect(pos)) return nullptr;

            auto cell = cells.find(pos);
            if (cell == cells.end()) return nullptr;
            return cell->second;
        }

        void setCell(QPoint pos, T *item) {
            if (!isInsideRect(pos)) return;

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
        class NearestAvailablePos {
        public:
            QPoint checkPos;
            QPoint basePos;

            NearestAvailablePos(QPoint checkPos, QPoint basePos) : checkPos(checkPos), basePos(basePos) {}

            float dist() const {
                auto distP = checkPos - basePos;
                return std::sqrt((float) distP.x() * distP.x() + distP.y() * distP.y());
            }

            bool operator<(const NearestAvailablePos &other) const {
                return dist() > other.dist();
            }
        };

        class CostPos {
        public:
            QPoint pos;
            QPoint dir;
            float priority;

            CostPos(QPoint pos, QPoint dir, float cost) : pos(pos), dir(dir), priority(cost) {}

            bool operator<(const CostPos &other) const {
                return priority > other.priority;
            }
        };

        class VisitedCell {
        public:
            QPoint from;
            float cost;

            VisitedCell(QPoint from, float cost) : from(from), cost(cost) {}
        };

        std::unordered_map<QPoint, T *> cells;
    };

}
