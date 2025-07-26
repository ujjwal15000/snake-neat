#include "Model/Model.h"
#include <memory>

Model::Model(int inputs, int outputs)
        : inputs_(inputs), outputs_(outputs) {

    // Helper
    auto createNode = [&](bool isInput, bool isHidden, ActivationType activationType) -> Node* {
        auto node = std::make_unique<Node>(id_++, isHidden, isInput, activationType);
        Node* nodePtr = node.get();
        nodes_.emplace(node->getId(), std::move(node));
        return nodePtr;
    };

    // === Create Layers ===
    std::vector<std::vector<Node*>> layers;

    // Input layer
    std::vector<Node*> inputLayer;
    for (int i = 0; i < inputs; ++i)
        inputLayer.push_back(createNode(true, false, ActivationType::Identity));
    layers.push_back(inputLayer);
    inputNodes_ = inputLayer;

//    // Hidden Layer 1: 24 neurons
//    std::vector<Node*> hidden1;
//    for (int i = 0; i < 24; ++i)
//        hidden1.push_back(createNode(false, true, ActivationType::Tanh));
//    layers.push_back(hidden1);

    // Hidden Layer 2: 32 neurons
//    std::vector<Node*> hidden2;
//    for (int i = 0; i < 16; ++i)
//        hidden2.push_back(createNode(false, true, ActivationType::Tanh));
//    layers.push_back(hidden2);

//    // Hidden layer 3: 16 neurons
//    std::vector<Node*> hidden3;
//    for (int i = 0; i < 16; ++i)
//        hidden3.push_back(createNode(false, true, ActivationType::ReLU));
//    layers.push_back(hidden3);

    // Output layer
    std::vector<Node*> outputLayer;
    for (int i = 0; i < outputs; ++i)
        outputLayer.push_back(createNode(false, false, ActivationType::Identity));
    layers.push_back(outputLayer);
    outputNodes_ = outputLayer;

    // === Connect Fully Between Layers ===
    for (size_t i = 0; i + 1 < layers.size(); ++i) {
        for (Node* from : layers[i]) {
            for (Node* to : layers[i + 1]) {
                addConnection(from, to);
            }
        }
    }
}


//Model::Model(int inputs, int outputs) : inputs_(inputs), outputs_(outputs) {
//    for (int i = 0; i < inputs; ++i) {
//        auto node = std::make_unique<Node>(id_++, false, true, ActivationType::Identity);
//        Node *nodePtr = node.get();
//        nodes_.emplace(node->getId(), std::move(node));
//        inputNodes_.push_back(nodePtr);
//    }
//
//    std::unordered_map<int, std::unique_ptr<Node>> outputNodes{};
//    for (int i = 0; i < outputs; ++i) {
//        auto outputNode = std::make_unique<Node>(id_++, false, false, ActivationType::Identity);
//        int id = outputNode->getId();
//
//        for (auto &[_, inputNode]: nodes_) {
//            addConnection(inputNode.get(), outputNode.get());
//        }
//
//        outputNodes.emplace(id, std::move(outputNode));
//    }
//
//    for (auto &[id, node]: outputNodes) {
//        Node *nodePtr = node.get();
//        nodes_.emplace(id, std::move(node));
//        outputNodes_.push_back(nodePtr);
//    }
//}

//Model::Model(int inputs, int outputs) : inputs_(inputs), outputs_(outputs) {
//    int layerSizes[] = {inputs, 16, 32, 16, 8, outputs};
//    std::vector<std::vector<Node*>> layers;
//
//    // Create nodes for each layer
//    for (int layer = 0; layer < 6; ++layer) {
//        std::vector<Node*> layerNodes;
//        bool isInput = (layer == 0);
//        bool isOutput = (layer == 5);
//
//        for (int i = 0; i < layerSizes[layer]; ++i) {
//            auto node = std::make_unique<Node>(id_++, !isInput && !isOutput, isInput);
//            Node *nodePtr = node.get();
//
//            if (isInput)
//                inputNodes_.push_back(nodePtr);
//            else if (isOutput)
//                outputNodes_.push_back(nodePtr);
//
//            nodes_.emplace(node->getId(), std::move(node));
//            layerNodes.push_back(nodePtr);
//
//            if (layer > 0 && layer < 5) {
//                nodePtr->setActivation(ActivationType::ReLU);
//            } else if (layer == 5) {
//                nodePtr->setActivation(ActivationType::Sigmoid);
//            }
//        }
//        layers.push_back(std::move(layerNodes));
//    }
//
//    // Fully connect layer[i] → layer[i + 1]
//    for (size_t i = 0; i < layers.size() - 1; ++i) {
//        for (Node *from : layers[i]) {
//            for (Node *to : layers[i + 1]) {
//                addConnection(from, to);
//            }
//        }
//    }
//}

void Model::addConnection(Node *from, Node *to) {
    auto conn = std::make_unique<Connection>(from->getId(), to->getId());
    connections_.emplace(std::make_pair(from->getId(), to->getId()), std::move(conn));
    from->addOut(to->getId());
    to->addIn(from->getId());
}

void Model::addConnection(double weight, Node *from, Node *to) {
    auto conn = std::make_unique<Connection>(weight, from->getId(), to->getId());
    connections_.emplace(std::make_pair(from->getId(), to->getId()), std::move(conn));
    from->addOut(to->getId());
    to->addIn(from->getId());
}

void Model::removeConnection(Connection *conn) {
    nodes_.at(conn->getFrom())->removeOut(conn->getTo());
    nodes_.at(conn->getTo())->removeIn(conn->getFrom());
    connections_.erase({conn->getFrom(), conn->getTo()});
}

void Model::addConnectionMutation() {
    Node *n1 = getRandomPair(nodes_).second.get();
    Node *n2 = getRandomPair(nodes_).second.get();

    if (connections_.find({n1->getId(), n2->getId()}) != connections_.end()) {
        connections_.at({n1->getId(), n2->getId()})->setEnabled(true);
        return;
    }

    if (checkCycle(&nodes_, n1->getId(), n2->getId()))
        return;

    addConnection(n1, n2);
}

void Model::removeConnectionMutation() {
    Connection *conn = getRandomPair(connections_).second.get();
    removeConnection(conn);
}

void Model::addNodeMutation() {
    Connection *conn = getRandomPair(connections_).second.get();
    double oldWeight = conn->getWeight();

    auto node = std::make_unique<Node>(id_++, true, false, ActivationType::Tanh);
    int nodeId = node->getId();
    nodes_.emplace(nodeId, std::move(node));

    addConnection(1.0, nodes_.at(conn->getFrom()).get(), nodes_.at(nodeId).get());
    addConnection(oldWeight, nodes_.at(nodeId).get(), nodes_.at(conn->getTo()).get());

    conn->setEnabled(false);
}

void Model::removeNodeMutation() {
    Node *node = getRandomPair(nodes_).second.get();
    if (!node->isHidden())
        return;

    for (int outId: node->getOut()) {
        removeConnection(connections_.at({node->getId(), outId}).get());
    }

    for (int inId: node->getIn()) {
        removeConnection(connections_.at({inId, node->getId()}).get());
    }

    nodes_.erase(node->getId());
}

bool Model::checkCycle(std::unordered_map<int, std::unique_ptr<Node> > *nodes, int from, int to) {
    std::unordered_set<int> visited;
    std::function<bool(int)> dfs = [&](int current) -> bool {
        if (current == from) return true;
        if (!visited.insert(current).second) return false;

        for (int neighborId: nodes->at(current)->getOut()) {
            if (dfs(neighborId)) return true;
        }
        return false;
    };
    return dfs(to);
}

std::vector<double> Model::feedForward(std::vector<double> &inputs) {
    if (inputs.size() != inputNodes_.size())
        throw std::invalid_argument("Input size mismatch");

    // Clear all node values before a new evaluation
    for (auto& [_, node] : nodes_) {
        node->setValue(0.0);
    }

    // Set input node values
    for (size_t i = 0; i < inputs.size(); ++i) {
        inputNodes_[i]->setValue(inputs[i]);
    }

    std::vector<double> outputs(outputNodes_.size());
    std::unordered_set<int> visited;

    std::function<void(Node *)> visit = [&](Node *node) {
        if (!visited.insert(node->getId()).second)
            return;

        for (int inId : node->getIn()) {
            visit(nodes_.at(inId).get());
        }

        if (!node->isInput()) {
            double sum = 0.0;
            for (int inId : node->getIn()) {
                Node *inNode = nodes_.at(inId).get();
                auto connIt = connections_.find({inId, node->getId()});
                if (connIt != connections_.end() && connIt->second->isEnabled()) {
                    double weight = connIt->second->getWeight();
                    sum += inNode->getValue() * weight;
                }
            }
            node->setValue(node->activate(sum + node->getBias()));
        }
    };

    // Start traversal from each output node
    for (auto *node : outputNodes_) {
        visit(node);
    }

    // Collect final output values
    for (size_t i = 0; i < outputNodes_.size(); ++i) {
        outputs[i] = outputNodes_[i]->getValue();
    }

    return outputs;
}


std::unique_ptr<Model> Model::crossover(Model *other) {
    Model *fitter = other->fitness_ > this->fitness_ ? other : this;
    Model *lessFitter = other->fitness_ <= this->fitness_ ? other : this;
    auto child = std::make_unique<Model>(inputs_, outputs_);

    for (auto &it: fitter->nodes_) {
        auto &currNode = it.second;
        auto otherNodeIt = lessFitter->nodes_.find(it.first);
        if (otherNodeIt != lessFitter->nodes_.end())
            child->nodes_.emplace(it.first, currNode->crossover(otherNodeIt->second.get()));
        else
            child->nodes_.emplace(it.first, std::make_unique<Node>(*currNode));
    }

    for (auto &it: fitter->connections_) {
        auto &currConnection = it.second;
        auto otherConnectionIt = lessFitter->connections_.find(it.first);
        if (otherConnectionIt != lessFitter->connections_.end())
            child->connections_.emplace(it.first, currConnection->crossover(otherConnectionIt->second.get()));
        else
            child->connections_.emplace(it.first, std::make_unique<Connection>(*currConnection));
    }

    return child;
}

void Model::mutate() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (auto &[id, node] : nodes_) {
        if (!node->isHidden()) continue;

        if (dist(rng_) < mutationConfig_.mutation_rate) {
            if (dist(rng_) < mutationConfig_.replace_rate) {
                node->setBias(newValue());
            } else {
                node->setBias(mutationDelta(node->getBias()));
            }
        }

//        if (dist(rng_) < 0.01) {
//            node->setActivation(Node::getRandomActivation());
//        }
    }

    for (auto &[key, conn] : connections_) {
        if (!conn->isEnabled()) continue;

        if (dist(rng_) < mutationConfig_.mutation_rate) {
            if (dist(rng_) < mutationConfig_.replace_rate) {
                conn->setWeight(newValue());
            } else {
                conn->setWeight(mutationDelta(conn->getWeight()));
            }
        }

        if (dist(rng_) < 0.01) {
            conn->setEnabled(!conn->isEnabled());
        }
    }

    if (dist(rng_) < 0.05) { addConnectionMutation(); }   // more links early on
    if (dist(rng_) < 0.05) { addNodeMutation(); }         // more structure growth
    if (dist(rng_) < 0.01) { removeConnectionMutation(); } // low but present
    if (dist(rng_) < 0.01) { removeNodeMutation(); }       // rare to avoid fragmentation

}

void Model::save(std::ostream& out) const {
    out.write(reinterpret_cast<const char*>(&inputs_), sizeof(inputs_));
    out.write(reinterpret_cast<const char*>(&outputs_), sizeof(outputs_));
    out.write(reinterpret_cast<const char*>(&id_), sizeof(id_));
    out.write(reinterpret_cast<const char*>(&fitness_), sizeof(fitness_));

    size_t nodeCount = nodes_.size();
    out.write(reinterpret_cast<const char*>(&nodeCount), sizeof(nodeCount));
    for (const auto& [id, node] : nodes_) {
        out.write(reinterpret_cast<const char*>(&id), sizeof(id));
        node->save(out);
    }

    size_t connCount = connections_.size();
    out.write(reinterpret_cast<const char*>(&connCount), sizeof(connCount));
    for (const auto& [key, conn] : connections_) {
        out.write(reinterpret_cast<const char*>(&key.first), sizeof(int));
        out.write(reinterpret_cast<const char*>(&key.second), sizeof(int));
        conn->save(out);
    }
}

void Model::load(std::istream& in) {
    in.read(reinterpret_cast<char*>(&inputs_), sizeof(inputs_));
    in.read(reinterpret_cast<char*>(&outputs_), sizeof(outputs_));
    in.read(reinterpret_cast<char*>(&id_), sizeof(id_));
    in.read(reinterpret_cast<char*>(&fitness_), sizeof(fitness_));

    size_t nodeCount;
    in.read(reinterpret_cast<char*>(&nodeCount), sizeof(nodeCount));
    nodes_.clear();
    inputNodes_.clear();
    outputNodes_.clear();
    for (size_t i = 0; i < nodeCount; ++i) {
        int id;
        in.read(reinterpret_cast<char*>(&id), sizeof(id));
        auto node = std::make_unique<Node>(0, false, false);
        node->load(in);
        if (node->isInput()) inputNodes_.push_back(node.get());
        else if (!node->isHidden()) outputNodes_.push_back(node.get());
        nodes_[id] = std::move(node);
    }

    size_t connCount;
    in.read(reinterpret_cast<char*>(&connCount), sizeof(connCount));
    connections_.clear();
    for (size_t i = 0; i < connCount; ++i) {
        int from, to;
        in.read(reinterpret_cast<char*>(&from), sizeof(from));
        in.read(reinterpret_cast<char*>(&to), sizeof(to));
        auto conn = std::make_unique<Connection>(0.0, from, to);
        conn->load(in);
        connections_[{from, to}] = std::move(conn);
    }
}

double Model::getCompatibilityDistance(Model *other) {
    const auto& conn1 = this->connections_;
    const auto& conn2 = other->connections_;

    std::unordered_set<std::pair<int, int>, PairHash> allKeys;
    int disjoint = 0;
    double weightDiffSum = 0.0;
    int matching = 0;

    for (const auto& [key, conn] : conn1) {
        allKeys.insert(key);
        auto it = conn2.find(key);
        if (it != conn2.end()) {
            // Matching gene
            weightDiffSum += std::abs(conn->getWeight() - it->second->getWeight());
            ++matching;
        } else {
            // Disjoint
            ++disjoint;
        }
    }

    for (const auto& [key, conn] : conn2) {
        if (allKeys.find(key) == allKeys.end()) {
            ++disjoint;
        }
    }

    // Normalize
    int N = std::max(conn1.size(), conn2.size());
    if (N < 20) N = 1;  // Avoid division by small values early on

    double avgWeightDiff = (matching > 0) ? (weightDiffSum / matching) : 0.0;

    // Coefficients (tune these)
//    double c1 = 1.0;  // disjoint
//    double c2 = 0.4;  // weight difference
    double c1 = 1.0;  // disjoint
    double c2 = 0.0;  // weight difference

    return (c1 * disjoint / N) + (c2 * avgWeightDiff);
}


std::unique_ptr<Model> Model::clone() const {
    auto cloned = std::make_unique<Model>(inputs_, outputs_);
    cloned->id_ = id_;
    cloned->fitness_ = fitness_;
    cloned->mutationConfig_ = mutationConfig_;

    // Clone nodes
    cloned->nodes_.clear();
    for (const auto &[id, node] : nodes_) {
        cloned->nodes_[id] = node->clone();
    }

    // Fix input/output pointers
    cloned->inputNodes_.clear();
    cloned->outputNodes_.clear();
    for (auto &[id, node] : cloned->nodes_) {
        if (node->isInput()) cloned->inputNodes_.push_back(node.get());
        else if (!node->isHidden()) cloned->outputNodes_.push_back(node.get());
    }

    // Clone connections
    cloned->connections_.clear();
    for (const auto &[key, conn] : connections_) {
        cloned->connections_[key] = conn->clone();
    }

    return cloned;
}

