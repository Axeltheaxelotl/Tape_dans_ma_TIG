# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: woody_woodpacker                          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/02                          #+#    #+#                  #
#    Updated: 2025/12/02                         ###   ########.fr            #
#                                                                              #
# **************************************************************************** #

# ============================================================================ #
#                                  VARIABLES                                   #
# ============================================================================ #

NAME		= woody_woodpacker

# Compilateur et flags
CC			= gcc
CFLAGS		= -Wall -Wextra -Werror -g3
NASM		= nasm
NASMFLAGS	= -f elf64

# Directories
SRC_DIR		= src
ASM_DIR		= asssrc
INC_DIR		= includes
LIBFT_DIR	= libft
OBJ_DIR		= obj
OBJ_ASM_DIR	= $(OBJ_DIR)/asm

# Libft
LIBFT		= $(LIBFT_DIR)/libft.a
LIBFT_INC	= $(LIBFT_DIR)

# Include paths
INCLUDES	= -I$(INC_DIR) -I$(LIBFT_INC)

# Source files
SRC_FILES	= main.c \
			  parsing.c \
			  error.c \
			  encryptitation_injectitation.c \
			  key_management.c \
			  smegma.c \
			  compression/rle_compress.c \
			  compression/rle_integration.c

ASM_FILES	= encryptitation.s \
			  decrypt_64.s \
			  decrypt_32.s \
			  rle_decompress_64.s \
			  rle_decompress_32.s

# Object files
SRC			= $(addprefix $(SRC_DIR)/, $(SRC_FILES))
OBJ			= $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.c=.o))
ASM_SRC		= $(addprefix $(ASM_DIR)/, $(ASM_FILES))
ASM_OBJ		= $(addprefix $(OBJ_ASM_DIR)/, $(ASM_FILES:.s=.o))

# Colors
GREEN		= \033[0;32m
RED			= \033[0;31m
YELLOW		= \033[0;33m
BLUE		= \033[0;34m
RESET		= \033[0m

# ============================================================================ #
#                                   RULES                                      #
# ============================================================================ #

all: $(NAME)

# Create directories
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_ASM_DIR)

# Compile libft
$(LIBFT):
	@echo "$(YELLOW)Compiling libft...$(RESET)"
	@$(MAKE) -C $(LIBFT_DIR) --no-print-directory
	@echo "$(GREEN)✓ Libft compiled$(RESET)"

# Compile C source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile assembly files
$(OBJ_ASM_DIR)/%.o: $(ASM_DIR)/%.s | $(OBJ_DIR)
	@echo "$(BLUE)Assembling $<...$(RESET)"
	@$(NASM) $(NASMFLAGS) $< -o $@

# Link everything
$(NAME): $(LIBFT) $(OBJ) $(ASM_OBJ)
	@echo "$(YELLOW)Linking $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJ) $(ASM_OBJ) -L$(LIBFT_DIR) -lft -o $(NAME)
	@echo "$(GREEN)✓ $(NAME) successfully compiled!$(RESET)"

# Clean object files
clean:
	@echo "$(RED)Cleaning object files...$(RESET)"
	@rm -rf $(OBJ_DIR)
	@$(MAKE) -C $(LIBFT_DIR) clean --no-print-directory
	@echo "$(GREEN)✓ Clean done$(RESET)"

# Clean everything
fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)
	@$(MAKE) -C $(LIBFT_DIR) fclean --no-print-directory
	@echo "$(GREEN)✓ Full clean done$(RESET)"

# Rebuild everything
re: fclean all

# Phony targets
.PHONY: all clean fclean re

# ============================================================================ #
#                                   BONUS                                      #
# ============================================================================ #

# Test targets (optional)
test: $(NAME)
	@echo "$(YELLOW)Running tests...$(RESET)"
	@./$(NAME) /bin/ls
	@echo "$(GREEN)✓ Test completed$(RESET)"

# RLE Compression Tests
RLE_TEST_SRC = rle_standalone.c \
               src/compression/rle_compress.c

RLE_ASM_OBJ = asssrc/rle_decompress_64.s

test-rle: $(RLE_TEST_SRC) $(RLE_ASM_OBJ)
	@echo "$(YELLOW)Compiling RLE test suite...$(RESET)"
	@$(NASM) $(NASMFLAGS) asssrc/rle_decompress_64.s -o obj/rle_decompress_64.o
	@$(CC) $(CFLAGS) $(INCLUDES) $(RLE_TEST_SRC) obj/rle_decompress_64.o -o test_rle
	@echo "$(GREEN)✓ RLE test compiled$(RESET)"
	@echo "$(YELLOW)Running RLE tests...$(RESET)"
	@./test_rle
	@echo "$(GREEN)✓ RLE tests completed$(RESET)"

test-rle-clean:
	@rm -f test_rle obj/rle_decompress_64.o
	@echo "$(GREEN)✓ RLE test files cleaned$(RESET)"

# Help
help:
	@echo "$(BLUE)Available targets:$(RESET)"
	@echo "  $(GREEN)all$(RESET)          - Build $(NAME)"
	@echo "  $(GREEN)clean$(RESET)        - Remove object files"
	@echo "  $(GREEN)fclean$(RESET)       - Remove object files and $(NAME)"
	@echo "  $(GREEN)re$(RESET)           - Rebuild everything"
	@echo "  $(GREEN)test$(RESET)         - Run basic test"
	@echo "  $(GREEN)test-rle$(RESET)     - Run RLE compression tests"
	@echo "  $(GREEN)test-rle-clean$(RESET) - Clean RLE test files"
	@echo "  $(GREEN)help$(RESET)         - Show this help message"

.PHONY: test help test-rle test-rle-clean
