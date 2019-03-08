#pragma once

#include <QtCore/QDataStream>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AudioBackend.h"

namespace AxiomBackend {

    template<class T>
    struct PersistentParameter {
        uint64_t id;
        size_t portalIndex;
        std::string name;

        PersistentParameter(uint64_t id, size_t portalIndex, std::string name)
            : id(id), portalIndex(portalIndex), name(std::move(name)) {}
    };

    using NumParameter = PersistentParameter<AxiomBackend::NumValue>;
    using MidiParameter = PersistentParameter<AxiomBackend::MidiValue>;

    template<class T>
    class PersistentParameters {
    public:
        const std::vector<std::optional<PersistentParameter<T>>> &parameters() const { return _parameters; }

        const std::unordered_map<size_t, size_t> &portalParameterMap() const { return _portalParameterMap; }

        size_t size() const { return _parameters.size(); }

        const std::optional<PersistentParameter<T>> &operator[](size_t index) const { return _parameters[index]; }

        void setParameters(std::vector<PersistentParameter<T>> newParameters) {
            // build a map of the portal ID to parameter index
            std::unordered_map<uint64_t, size_t> parameterIndexMap;
            for (size_t parameterIndex = 0; parameterIndex < _parameters.size(); parameterIndex++) {
                auto &parameter = _parameters[parameterIndex];
                if (parameter) {
                    parameterIndexMap.emplace(parameter->id, parameterIndex);
                }
            }

            _parameters.clear();
            _portalParameterMap.clear();
            std::vector<PersistentParameter<T>> queuedParameters;

            for (auto &newParameter : newParameters) {
                // if the parameter had a previous index, insert it there, else queue it to be inserted later
                auto previousParameterIndex = parameterIndexMap.find(newParameter.id);
                if (previousParameterIndex != parameterIndexMap.end()) {
                    insertParameter(previousParameterIndex->second, std::move(newParameter));
                } else {
                    queuedParameters.push_back(std::move(newParameter));
                }
            }

            // go through and insert the remaining parameters
            for (auto &queuedParam : queuedParameters) {
                pushParameter(std::move(queuedParam));
            }
        }

        void serialize(QDataStream &stream) const {
            stream << (uint32_t) _parameters.size();
            for (const auto &param : _parameters) {
                if (param) {
                    stream << (qint64) param->id;
                } else {
                    stream << (qint64) -1;
                }
            }
        }

        static PersistentParameters deserialize(QDataStream &stream, uint32_t version) {
            // PersistentParameters didn't exist before 0.4.0 (corresponding to schema version 5)
            if (version < 5) {
                return PersistentParameters();
            }

            quint32 parameterCount;
            stream >> parameterCount;

            PersistentParameters parameters;
            for (uint32_t paramIndex = 0; paramIndex < parameterCount; paramIndex++) {
                qint64 paramId;
                stream >> paramId;

                if (paramId == -1) {
                    parameters._parameters.push_back(std::nullopt);
                } else {
                    parameters._parameters.push_back(PersistentParameter<T>((uint64_t) paramId, 0, ""));
                }
            }
            return parameters;
        }

    private:
        std::vector<std::optional<PersistentParameter<T>>> _parameters;
        std::unordered_map<size_t, size_t> _portalParameterMap;

        void insertParameter(size_t insertIndex, PersistentParameter<T> param) {
            while (_parameters.size() <= insertIndex) _parameters.emplace_back(std::nullopt);
            _parameters[insertIndex] = std::move(param);
            _portalParameterMap.emplace(param.portalIndex, insertIndex);
        }

        void pushParameter(PersistentParameter<T> param) {
            // loop until we find an available index
            size_t nextIndex = 0;
            while (nextIndex < _parameters.size() && _parameters[nextIndex]) {
                nextIndex++;
            }
            insertParameter(nextIndex, std::move(param));
        }
    };

    using NumParameters = PersistentParameters<AxiomBackend::NumValue>;
    using MidiParameters = PersistentParameters<AxiomBackend::MidiValue>;
}
