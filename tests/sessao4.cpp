#include "../source/BehaviorTree.hpp"

#include <iostream>
#include <string>

static int failures = 0;

static void expect_eq(const std::string& got, const std::string& want, const char* name) {
    if (got != want) {
        std::cerr << "FAIL " << name << " got=\"" << got << "\" want=\"" << want << "\"\n";
        ++failures;
    }
}

static void expect(bool cond, const char* name) {
    if (!cond) {
        std::cerr << "FAIL " << name << "\n";
        ++failures;
    }
}

static BehaviorTree makeDoorTree() {
    return BehaviorTree(
        Selector(
            Sequence(Condition("Passou"), Action("FechePorta")),
            Sequence(Condition("NaPorta"), Condition("PortaAberta"), Action("Passe")),
            Sequence(Condition("NaPorta"), Condition("PortaDestrancada"), Condition("PortaFechada"), Action("AbrirPorta")),
            Sequence(Condition("NaPorta"), Condition("PortaTrancada"), Action("DestranquePorta")),
            Action("CaminhePorta")
        )
    );
}

static BehaviorTree makePatrolTree() {
    return BehaviorTree(
        Selector(
            Sequence(Condition("ChegouPontoA"), Action("InverterDirecao")),
            Sequence(Condition("ChegouPontoB"), Action("InverterDirecao")),
            Sequence(Condition("IndoParaA"), Action("AndeParaA")),
            Sequence(Condition("IndoParaB"), Action("AndeParaB"))
        )
    );
}

int main() {
    BehaviorTree door = makeDoorTree();

    expect_eq(door.tick({{"NaPorta", 1}, {"PortaTrancada", 1}, {"PortaFechada", 1}}),
              "DestranquePorta", "teste.cpp door locked");
    expect_eq(door.tick({}), "CaminhePorta", "empty context walks");
    expect_eq(door.tick({{"Passou", 1}}), "FechePorta", "passed closes");
    expect_eq(door.tick({{"NaPorta", 1}, {"PortaAberta", 1}}), "Passe", "open door pass");
    expect_eq(door.tick({{"NaPorta", 1}, {"PortaDestrancada", 1}, {"PortaFechada", 1}}),
              "AbrirPorta", "unlocked closed opens");

    BehaviorTree condTree(Condition("Flag"));
    expect_eq(condTree.tick({{"Flag", 0}}), "", "present zero fails");
    expect_eq(condTree.tick({}), "", "absent key fails");
    expect_eq(condTree.tick({{"Flag", 1}}), "", "true condition no command");

    BehaviorTree emptySelector(Selector());
    expect_eq(emptySelector.tick({}), "", "selector no children");

    // Sequence with no children: vacuously succeeds; last non-empty command is "".
    SequenceNode emptySeq(std::vector<BTNodePtr>{});
    const BTResult emptySeqResult = emptySeq.tick({});
    expect(emptySeqResult.success && emptySeqResult.command.empty(),
           "empty sequence succeeds with empty command");
    BehaviorTree emptySeqTree(Sequence());
    expect_eq(emptySeqTree.tick({}), "", "empty sequence tree tick");

    BehaviorTree lastAction(
        Sequence(Action("Primeiro"), Action(""), Action("Ultimo"))
    );
    expect_eq(lastAction.tick({}), "Ultimo", "sequence last non-empty command");

    BehaviorTree patrol = makePatrolTree();
    expect_eq(patrol.tick({{"IndoParaA", 1}}), "AndeParaA", "patrol to A");
    expect_eq(patrol.tick({{"IndoParaB", 1}}), "AndeParaB", "patrol to B");
    expect_eq(patrol.tick({{"ChegouPontoA", 1}}), "InverterDirecao", "arrived A");
    expect_eq(patrol.tick({{"ChegouPontoB", 1}}), "InverterDirecao", "arrived B");

    if (failures != 0) {
        std::cerr << failures << " failed\n";
        return 1;
    }
    std::cout << "ok sessao4\n";
    return 0;
}
