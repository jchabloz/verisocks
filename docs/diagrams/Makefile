# PlantUML diagrams makefile

OUT_DIR = ../_static/diagrams
diagrams = \
	verisocks_architecture.puml \
	verisocks_architecture_vsl.puml \
	verisocks_exec_focus.puml

JAVA = /usr/bin/java
PLANTUML_JAR = $(HOME)/Downloads/install/plantuml.jar
#PLANTUML_FLAGS =

svg_files = $(addprefix $(OUT_DIR)/,$(patsubst %.puml,%.svg,$(diagrams)))

all: $(svg_files)

$(OUT_DIR)/%.svg: %.puml
	@mkdir -p $(OUT_DIR)
	$(JAVA) -jar $(PLANTUML_JAR) -tsvg -o "$(OUT_DIR)" $(PLANTUML_FLAGS) $<

.PHONY: all clean

clean:
	$(RM) $(svg_files)

