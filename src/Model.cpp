#include "Model/Model.h"
#include <memory>

Model::Model(int inputs, int outputs) : inputs_(inputs), outputs_(outputs) {
    for (int i = 0; i < inputs; ++i) {
        auto node = std::make_unique<Node>(id_++, false, true);
        Node *nodePtr = node.get();
        nodes_.emplace(node->getId(), std::move(node));
        inputNodes_.push_back(nodePtr);
    }

    std::unordered_map<int, std::unique_ptr<Node>> outputNodes{};
    for (int i = 0; i < outputs; ++i) {
        auto outputNode = std::make_unique<Node>(id_++, false, false);
        int id = outputNode->getId();

        for (auto &[_, inputNode]: nodes_) {
            addConnection(inputNode.get(), outputNode.get());
        }

        outputNodes.emplace(id, std::move(outputNode));
    }

    for (auto &[id, node]: outputNodes) {
        Node *nodePtr = node.get();
        nodes_.emplace(id, std::move(node));
        outputNodes_.push_back(nodePtr);
    }
}

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

    auto node = std::make_unique<Node>(id_++, true, false);
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

std::vector<double> &Model::feedForward(std::vector<double> &inputs) {
    if (inputs.size() != inputNodes_.size())
        throw std::invalid_argument("Input size mismatch");

    // 1. Set input values
    for (size_t i = 0; i < inputs.size(); ++i) {
        inputNodes_[i]->setValue(inputs[i]);
    }

    // 2. Prepare to store output values
    static std::vector<double> outputs(outputNodes_.size());

    // 3. Do a simple topological order by layering:
    std::unordered_set<int> visited;
    std::function<void(Node *)> visit = [&](Node *node) {
        if (!visited.insert(node->getId()).second)
            return;
        for (int inId: node->getIn())
            visit(nodes_.at(inId).get());

        if (!node->isInput()) {
            double sum = 0.0;
            for (int inId: node->getIn()) {
                Node *inNode = nodes_.at(inId).get();
                double weight = 0.0;
                if (connections_.at({inId, node->getId()})->isEnabled())
                    weight = connections_.at({inId, node->getId()})->getWeight();
                sum += inNode->getValue() * weight;
            }
            node->setValue(node->activate(sum + node->getBias()));
        }
    };

    // 4. Visit all output nodes (which recursively visits everything)
    for (auto *node: outputNodes_) {
        visit(node);
    }

    // 5. Collect outputs
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
        if (node->isInput()) continue;

        if (dist(rng_) < mutationConfig_.mutation_rate) {
            if (dist(rng_) < mutationConfig_.replace_rate) {
                node->setBias(newValue());
            } else {
                node->setBias(mutationDelta(node->getBias()));
            }
        }
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

    if (dist(rng_) < 0.05) {
        addConnectionMutation();
    }

    if (dist(rng_) < 0.03) {
        addNodeMutation();
    }

    if (dist(rng_) < 0.01) {
        removeConnectionMutation();
    }

    if (dist(rng_) < 0.01) {
        removeNodeMutation();
    }
}

