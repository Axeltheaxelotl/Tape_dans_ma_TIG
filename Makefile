NAME		= woody_woodpacker

CC			= gcc
CFLAGS		= -Wall -Wextra -Werror -g3
NASM		= nasm
NASMFLAGS	= -f elf64

SRC_DIR		= src
ASM_DIR		= asssrc
PE_DIR		= pe
INC_DIR		= includes
LIBFT_DIR	= libft
OBJ_DIR		= obj
OBJ_ASM_DIR	= $(OBJ_DIR)/asm
OBJ_PE_DIR	= $(OBJ_DIR)/pe

LIBFT		= $(LIBFT_DIR)/libft.a
LIBFT_INC	= $(LIBFT_DIR)

INCLUDES	= -I$(INC_DIR) -I$(LIBFT_INC) -I$(PE_DIR)

SRC_FILES	= main.c \
			  parsing.c \
			  error.c \
			  encryptitation_injectitation.c \
			  key_management.c \
			  smegma.c

PE_FILES	= pe_parser.c \
			  pe_encryptitation.c \
			  pe_injectitation.c

ASM_FILES	= encryptitation.s \
			  decrypt_64.s \
			  decrypt_32.s

SRC			= $(addprefix $(SRC_DIR)/, $(SRC_FILES))
OBJ			= $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.c=.o))
PE_SRC		= $(addprefix $(PE_DIR)/, $(PE_FILES))
PE_OBJ		= $(addprefix $(OBJ_PE_DIR)/, $(PE_FILES:.c=.o))
ASM_SRC		= $(addprefix $(ASM_DIR)/, $(ASM_FILES))
ASM_OBJ		= $(addprefix $(OBJ_ASM_DIR)/, $(ASM_FILES:.s=.o))

GREEN		= \033[0;32m
RED			= \033[0;31m
YELLOW		= \033[0;33m
BLUE		= \033[0;34m
RESET		= \033[0m

all: $(NAME)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_ASM_DIR)
	@mkdir -p $(OBJ_PE_DIR)

$(LIBFT):
	@echo "$(YELLOW)Compiling libft...$(RESET)"
	@$(MAKE) -C $(LIBFT_DIR) --no-print-directory
	@echo "$(GREEN)✓ Libft compiled$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_PE_DIR)/%.o: $(PE_DIR)/%.c | $(OBJ_DIR)
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_ASM_DIR)/%.o: $(ASM_DIR)/%.s | $(OBJ_DIR)
	@echo "$(BLUE)Assembling $<...$(RESET)"
	@$(NASM) $(NASMFLAGS) $< -o $@

$(NAME): $(LIBFT) $(OBJ) $(PE_OBJ) $(ASM_OBJ)
	@echo "$(YELLOW)Linking $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJ) $(PE_OBJ) $(ASM_OBJ) -L$(LIBFT_DIR) -lft -o $(NAME)
	@echo "$(GREEN)✓ $(NAME) successfully compiled!$(RESET)"

clean:
	@echo "$(RED)Cleaning object files...$(RESET)"
	@rm -rf $(OBJ_DIR)
	@$(MAKE) -C $(LIBFT_DIR) clean --no-print-directory
	@echo "$(GREEN)✓ Clean done$(RESET)"

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)
	@rm -f woody
	@rm -f a.out
	@rm -f pe_test
	@$(MAKE) -C $(LIBFT_DIR) fclean --no-print-directory
	@echo "$(GREEN)✓ Full clean done$(RESET)"

re: fclean all

.PHONY: all clean fclean re test test-rle test-rle-clean clean-tests help

clean-tests:
	@echo "$(RED)Cleaning test binaries...$(RESET)"
	@rm -f test_compressible test_nops test_simple test_rle test_32bit woody pe_test pe_generator *.exe
	@rm -rf pe/pe_samples
	@echo "$(GREEN)✓ Test binaries cleaned$(RESET)"

test: $(NAME)
	@echo "$(YELLOW)Running tests...$(RESET)"
	@./$(NAME) /bin/ls
	@echo "$(GREEN)✓ Test completed$(RESET)"

# Test PE: programme pour analyser des fichiers PE sur Linux
pe_test: $(LIBFT) | $(OBJ_DIR)
	@echo "$(YELLOW)Compiling pe_test...$(RESET)"
	@mkdir -p $(OBJ_PE_DIR)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(PE_DIR)/pe_parser.c -o $(OBJ_PE_DIR)/pe_parser.o
	@$(CC) $(CFLAGS) $(INCLUDES) $(PE_DIR)/pe_test.c $(OBJ_PE_DIR)/pe_parser.o -L$(LIBFT_DIR) -lft -o pe_test
	@echo "$(GREEN)✓ pe_test compiled! Usage: ./pe_test <fichier.exe>$(RESET)"

# Générateur de PE: crée des fichiers PE de test
pe_generator: $(LIBFT) | $(OBJ_DIR)
	@echo "$(YELLOW)Compiling pe_generator...$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) $(PE_DIR)/pe_generator.c -L$(LIBFT_DIR) -lft -o pe_generator
	@echo "$(GREEN)✓ pe_generator compiled! Usage: ./pe_generator [32|64] <output.exe>$(RESET)"

help:
	@echo "$(BLUE)Available targets:$(RESET)"
	@echo "  $(GREEN)all$(RESET)           - Build $(NAME)"
	@echo "  $(GREEN)clean$(RESET)         - Remove object files"
	@echo "  $(GREEN)fclean$(RESET)        - Remove object files and $(NAME)"
	@echo "  $(GREEN)re$(RESET)            - Rebuild everything"
	@echo "  $(GREEN)test$(RESET)          - Run basic test"
	@echo "  $(GREEN)pe_test$(RESET)       - Build PE file analyzer"
	@echo "  $(GREEN)pe_generator$(RESET)  - Build PE file generator"
	@echo "  $(GREEN)clean-tests$(RESET)   - Clean all test binaries"
	@echo "  $(GREEN)help$(RESET)          - Show this help message"

.PHONY: test help clean-tests pe_test pe_generator

