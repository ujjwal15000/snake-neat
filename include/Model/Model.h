#pragma once

#include <unordered_map>
#include <unordered_set>
#include <Utils/RandomUtils.h>
#include <Utils/MutationUtils.h>
#include <ostream>
#include <istream>


enum class ActivationType {
    Identity,
    Sigmoid,
    ReLU,
    Tanh
};

struct Node {
    Node(int id, bool hidden, bool input)
            : id_(id),
              bias_(input ? 0.0 : newValue()),
              hidden_(hidden), input_(input),
              activationType_(getRandomActivation()) {}

    Node(int id, bool hidden, bool input, double bias, ActivationType activationType)
            : id_(id),
              bias_(bias),
              hidden_(hidden), input_(input),
              activationType_(activationType) {}

    Node(int id, bool hidden, bool input, ActivationType activationType)
            : id_(id),
              bias_(input ? 0.0 : newValue()),
              hidden_(hidden), input_(input),
              activationType_(activationType) {}

    Node(const Node &other) = default;

    void addIn(int i) { in_.insert(i); }

    void removeIn(int i) { in_.erase(i); }

    void addOut(int i) { out_.insert(i); }

    void removeOut(int i) { out_.erase(i); }

    void setValue(double value) { value_ = value; }

    void setBias(double bias) { bias_ = bias; }

    void setActivation(ActivationType activationType) { activationType_ = activationType; }

    double getValue() const { return value_; }

    bool isHidden() const { return hidden_; }

    bool isInput() const { return input_; }

    std::unique_ptr<Node> crossover(const Node *other) {
        // maybe add id check
        double bias = pickRandom(this->bias_, other->bias_);
        auto activation = pickRandom(this->activationType_, other->activationType_);
        return std::make_unique<Node>(this->getId(), false, false, bias, activation);
    }

    double activate(double input) {
        switch (activationType_) {
            case ActivationType::Identity:
                return input;
            case ActivationType::Sigmoid:
                return 1.0 / (1.0 + std::exp(-input));
            case ActivationType::ReLU:
                return input > 0 ? input : 0;
            case ActivationType::Tanh:
                return std::tanh(input);
            default:
                return input;
        }
    }

    static ActivationType getRandomActivation() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dist(0, 3);  // 4 types

        return static_cast<ActivationType>(dist(gen));
    }

    [[nodiscard]] int getId() const { return id_; }

    [[nodiscard]] double getBias() const { return bias_; }

    [[nodiscard]] std::unordered_set<int> getIn() const { return in_; }

    [[nodiscard]] std::unordered_set<int> getOut() const { return out_; }

    void save(std::ostream& out) const {
        out.write(reinterpret_cast<const char*>(&id_), sizeof(id_));
        out.write(reinterpret_cast<const char*>(&bias_), sizeof(bias_));
        out.write(reinterpret_cast<const char*>(&hidden_), sizeof(hidden_));
        out.write(reinterpret_cast<const char*>(&input_), sizeof(input_));
        auto act = static_cast<int>(activationType_);
        out.write(reinterpret_cast<const char*>(&act), sizeof(act));

        size_t inSize = in_.size();
        out.write(reinterpret_cast<const char*>(&inSize), sizeof(inSize));
        for (int i : in_) out.write(reinterpret_cast<const char*>(&i), sizeof(i));

        size_t outSize = out_.size();
        out.write(reinterpret_cast<const char*>(&outSize), sizeof(outSize));
        for (int i : out_) out.write(reinterpret_cast<const char*>(&i), sizeof(i));
    }

    void load(std::istream& in) {
        int act;
        in.read(reinterpret_cast<char*>(&id_), sizeof(id_));
        in.read(reinterpret_cast<char*>(&bias_), sizeof(bias_));
        in.read(reinterpret_cast<char*>(&hidden_), sizeof(hidden_));
        in.read(reinterpret_cast<char*>(&input_), sizeof(input_));
        in.read(reinterpret_cast<char*>(&act), sizeof(act));
        activationType_ = static_cast<ActivationType>(act);

        size_t inSize;
        in.read(reinterpret_cast<char*>(&inSize), sizeof(inSize));
        in_.clear();
        for (size_t i = 0; i < inSize; ++i) {
            int v;
            in.read(reinterpret_cast<char*>(&v), sizeof(v));
            in_.insert(v);
        }

        size_t outSize;
        in.read(reinterpret_cast<char*>(&outSize), sizeof(outSize));
        out_.clear();
        for (size_t i = 0; i < outSize; ++i) {
            int v;
            in.read(reinterpret_cast<char*>(&v), sizeof(v));
            out_.insert(v);
        }
    }

    std::unique_ptr<Node> clone() const {
        auto cloned = std::make_unique<Node>(id_, hidden_, input_, bias_, activationType_);
        cloned->in_ = in_;
        cloned->out_ = out_;
        cloned->value_ = value_;
        return cloned;
    }

private:
    int id_;
    double bias_, value_{0.0};
    bool hidden_, input_;
    ActivationType activationType_;
    std::unordered_set<int> out_{}, in_{};
};

struct Connection {
    Connection(double weight, int from, int to)
            : weight_(weight), from_(from), to_(to), enabled_(true) {}

    Connection(int from, int to)
            : weight_(newValue()), from_(from), to_(to), enabled_(true) {}

    Connection(const Connection &other) = default;

    double getWeight() const { return weight_; }

    void setWeight(double weight) { weight_ = weight; }

    int getFrom() const { return from_; }

    int getTo() const { return to_; }

    bool isEnabled() const { return enabled_; }

    void setEnabled(bool enabled) { enabled_ = enabled; }

    std::unique_ptr<Connection> crossover(const Connection *other) {
        // maybe add id check
        double weight = pickRandom(this->weight_, other->weight_);
        auto conn = std::make_unique<Connection>(weight, this->getFrom(), this->getTo());
        conn->setEnabled(pickRandom(this->enabled_, other->enabled_));
        return conn;
    }

    void save(std::ostream& out) const {
        out.write(reinterpret_cast<const char*>(&weight_), sizeof(weight_));
        out.write(reinterpret_cast<const char*>(&from_), sizeof(from_));
        out.write(reinterpret_cast<const char*>(&to_), sizeof(to_));
        out.write(reinterpret_cast<const char*>(&enabled_), sizeof(enabled_));
    }

    void load(std::istream& in) {
        in.read(reinterpret_cast<char*>(&weight_), sizeof(weight_));
        in.read(reinterpret_cast<char*>(&from_), sizeof(from_));
        in.read(reinterpret_cast<char*>(&to_), sizeof(to_));
        in.read(reinterpret_cast<char*>(&enabled_), sizeof(enabled_));
    }

    std::unique_ptr<Connection> clone() const {
        auto conn = std::make_unique<Connection>(weight_, from_, to_);
        conn->setEnabled(enabled_);
        return conn;
    }


private:
    double weight_;
    int from_, to_;
    bool enabled_;
};

class Model {
public:
    Model(int inputs, int outputs);

    std::vector<double> feedForward(std::vector<double> &inputs);

    void setFitness(double fitness) { fitness_ = fitness; }

    void mutate();

    std::unique_ptr<Model> crossover(Model *other);

    void save(std::ostream& out) const;
    void load(std::istream& in);
    double getCompatibilityDistance(Model *other);
    std::unique_ptr<Model> clone() const;

private:
    int inputs_, outputs_, id_ = 0;
    double fitness_{0.0};
    std::unordered_map<int, std::unique_ptr<Node>> nodes_{};
    std::vector<Node *> inputNodes_{}, outputNodes_{};
    std::unordered_map<std::pair<int, int>, std::unique_ptr<Connection>, PairHash> connections_{};
    DoubleConfig mutationConfig_{};
    std::mt19937 rng_{std::random_device{}()};

    void addConnection(Node *from, Node *to);

    void addConnection(double weight, Node *from, Node *to);

    void removeConnection(Connection *connection);

    void addConnectionMutation();

    void removeConnectionMutation();

    void addNodeMutation();

    void removeNodeMutation();

    static bool checkCycle(std::unordered_map<int, std::unique_ptr<Node> > *nodes, int from, int to);
};