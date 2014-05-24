all: tests

tests:
	@$(MAKE) --no-print-directory -f API/common.mk PROJECT=projects/tests/tests.mk all
