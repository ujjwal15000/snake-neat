#pragma once

#include <unordered_map>
#include <unordered_set>
#include <Utils/RandomUtils.h>

struct Node {
    Node(int id, bool hidden, bool input)
            : id_(id), bias_(input ? 0.0 : randomDouble01()), hidden_(hidden), input_(input) {}

    void addIn(int i) { in_.insert(i); }

    void removeIn(int i) { in_.erase(i); }

    void addOut(int i) { out_.insert(i); }

    void removeOut(int i) { out_.erase(i); }

    void setValue(double value) { value_ = value; }

    double getValue() const { return value_; }

    bool isHidden() const { return hidden_; }

    bool isInput() const { return input_; }

    static double activate(double input) { return std::tanh(input); }

    [[nodiscard]] int getId() const { return id_; }

    [[nodiscard]] double getBias() const { return bias_; }

    [[nodiscard]] std::unordered_set<int> getIn() const { return in_; }

    [[nodiscard]] std::unordered_set<int> getOut() const { return out_; }

private:
    int id_;
    double bias_, value_{0.0};
    bool hidden_, input_;
    std::unordered_set<int> out_{}, in_{};
};

struct Connection {
    Connection(double weight, int from, int to)
            : weight_(weight), from_(from), to_(to), enabled_(true) {}

    Connection(int from, int to)
            : weight_(randomDouble01()), from_(from), to_(to), enabled_(true) {}

    double getWeight() const { return weight_; }

    int getFrom() const { return from_; }

    int getTo() const { return to_; }

    bool isEnabled() const { return enabled_; }

    void setEnabled(bool enabled) { enabled_ = enabled; }

private:
    double weight_;
    int from_;
    int to_;
    bool enabled_;
};

class Model {
public:
    Model(int inputs, int outputs);

    std::vector<double> &feedForward(std::vector<double> &inputs);

private:
    int inputs_, outputs_, id_ = 0;
    std::unordered_map<int, std::unique_ptr<Node>> nodes_{};
    std::vector<Node*> inputNodes_{}, outputNodes_{};
    std::unordered_map<std::pair<int, int>, std::unique_ptr<Connection>, PairHash> connections_{};

    void addConnection(Node *from, Node *to);
    void addConnection(double weight, Node *from, Node *to);
    void removeConnection(Connection* connection);

    void addConnectionMutation();

    void removeConnectionMutation();

    void addNodeMutation();

    void removeNodeMutation();

    static bool checkCycle(std::unordered_map<int, std::unique_ptr<Node> > *nodes, int from, int to);
};