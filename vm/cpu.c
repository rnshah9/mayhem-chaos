/*
 * Description: CPU module of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2021 Chaos Language Development Authority <info@chaos-lang.org>
 *
 * License: GNU General Public License v3.0
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>
 *
 * Authors: M. Mert Yildiran <me@mertyildiran.com>
 */

#include "cpu.h"

char *reg_names[] = {
    "R0A", "R1A", "R2A", "R3A", "R4A", "R5A", "R6A", "R7A",
    "R0B", "R1B", "R2B", "R3B", "R4B", "R5B", "R6B", "R7B"
};

cpu *new_cpu(i64 *memory, i64 mem_size, bool debug)
{
	cpu *c = malloc(sizeof(cpu));
	c->mem = memory;
	c->sp = mem_size - 1;
	c->max_mem = mem_size;
	c->pc = -1;
	c->inst = 0;
    c->debug = debug;
	return c;
}

void free_cpu(cpu *c)
{
	free(c);
}

void run_cpu(cpu *c)
{
	while (c->inst != HLT) {
		fetch(c);
		execute(c);
	}
}

void fetch(cpu *c)
{
	c->pc++;
	c->inst = c->mem[c->pc];
	c->dest = c->mem[c->pc+1];
	c->src = c->mem[c->pc+2];
}

void execute(cpu *c)
{
    size_t len;

	switch (c->inst) {
    case CLF:
        clear_flags(c);
        break;
    case CMP:
        if (c->r[R0A] == V_FLOAT) {
            f64 f1;
            f64 f2;
            get_f64_operands(c, &f1, &f2);
            fset_flags(c, f1, f2);
        } else {
            set_flags(c, c->r[c->dest], c->r[c->src]);
        }
        c->pc += 2;
        break;
    case CMPI:
        set_flags(c, c->r[c->dest], c->src);
        c->pc += 2;
        break;
    case MOV:
        c->r[c->dest] = c->r[c->src];
        c->pc += 2;
        break;
    case STI:
        c->mem[c->dest] = c->r[c->src];
        c->pc += 2;
        break;
    case LDI:
        c->r[c->dest] = c->mem[c->src];
        c->pc += 2;
        break;
    case LII:
        c->r[c->dest] = c->src;
        c->pc += 2;
        break;
    case PUSH:
        c->mem[--c->sp] = c->r[c->mem[++c->pc]];
        break;
    case POP:
        c->r[c->mem[++c->pc]] = c->mem[c->sp++];
        break;
    case INC:
        c->r[c->dest]++;
        c->pc++;
        break;
    case DEC:
        c->r[c->dest]--;
        c->pc++;
        break;
    case ADD:
        if (c->r[R0A] == V_FLOAT) {
            f64 f1;
            f64 f2;
            get_f64_operands(c, &f1, &f2);
            f1 = f1 + f2;
            load_f64(c, f1);
        } else {
            c->r[c->dest] += c->r[c->src];
        }
        c->pc += 2;
        break;
    case SUB:
        if (c->r[R0A] == V_FLOAT) {
            f64 f1;
            f64 f2;
            get_f64_operands(c, &f1, &f2);
            f1 = f1 - f2;
            load_f64(c, f1);
        } else {
            c->r[c->dest] -= c->r[c->src];
        }
        c->pc += 2;
        break;
    case MUL:
        if (c->r[R0A] == V_FLOAT) {
            f64 f1;
            f64 f2;
            get_f64_operands(c, &f1, &f2);
            f1 = f1 * f2;
            load_f64(c, f1);
        } else {
            c->r[c->dest] *= c->r[c->src];
        }
        c->pc += 2;
        break;
    case DIV:
        if (c->r[R0A] == V_FLOAT) {
            f64 f1;
            f64 f2;
            get_f64_operands(c, &f1, &f2);
            f1 = f1 / f2;
            load_f64(c, f1);
        } else {
            c->r[c->dest] /= c->r[c->src];
        }
        c->pc += 2;
        break;
    case MOD:
        if (c->r[R0A] == V_FLOAT) {
            f64 f1;
            f64 f2;
            get_f64_operands(c, &f1, &f2);
            f1 = fmodl(f1, f2);
            load_f64(c, f1);
        } else {
            c->r[c->dest] %= c->r[c->src];
        }
        c->pc += 2;
        break;
    case JLZ:
        if (c->ltz) {
            ++(c->pc);
            c->pc = c->mem[c->pc];
        }
        else c->pc++;
        break;
    case JGZ:
        if (c->gtz) {
            ++(c->pc);
            c->pc = c->mem[c->pc];
        }
        else c->pc++;
        break;
    case JEZ:
        if (c->zero) {
            ++(c->pc);
            c->pc = c->mem[c->pc];
        }
        else c->pc++;
        break;
    case JNZ:
        if (!c->zero) {
            ++(c->pc);
            c->pc = c->mem[c->pc];
        }
        else c->pc++;
        break;
    case JMP:
        ++(c->pc);
        c->pc = c->mem[c->pc];
        break;
    case SHL:
        c->r[c->dest] <<= c->r[c->src];
        c->pc += 2;
        break;
    case SHR:
        c->r[c->dest] >>= c->r[c->src];
        c->pc += 2;
        break;
    case BAND:
        c->r[c->dest] &= c->r[c->src];
        c->pc += 2;
        break;
    case BOR:
        c->r[c->dest] |= c->r[c->src];
        c->pc += 2;
        break;
    case BNOT:
        c->r[c->dest] = ~c->r[c->dest];
        c->pc++;
        break;
    case BXOR:
        c->r[c->dest] ^= c->r[c->src];
        c->pc += 2;
        break;
    case LAND:
        c->r[c->dest] = c->r[c->dest] && c->r[c->src];
        c->pc += 2;
        break;
    case LOR:
        c->r[c->dest] = c->r[c->dest] || c->r[c->src];
        c->pc += 2;
        break;
    case LNOT:
        c->r[c->dest] = !c->r[c->dest];
        c->pc++;
        break;
    case PRNT:
        switch (c->r[R0A]) {
        case V_BOOL:
            printf("%s\n", c->r[R1A] ? "true" : "false");
            break;
        case V_INT:
            printf("%lld\n", c->r[R1A]);
            break;
        case V_FLOAT:
            printf("%Lg\n", build_f64(c->r[R1A], c->r[R2A], c->r[R3A]));
            break;
        case V_STRING:
            len = c->r[R1A];
            for (size_t i = 0; i < len; i++) {
                c->r[R1A] = c->mem[c->sp++];
                printf("%c", (int)c->r[R1A] + '0');
            }
            printf("\n");
            break;
        default:
            break;
        }
        break;
    default:
        break;
	}

    if (c->debug)
        print_registers(c);
}

void print_registers(cpu *c)
{
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("[%s: %lld] ", getRegName(i), c->r[i]);
    }
    printf("\n");
}

char *getRegName(i64 i)
{
    return reg_names[i];
}

f64 build_f64(i64 ipart, i64 frac, i64 leading_zeros)
{
    int sign = (ipart < 0) ? -1 : 1;
    ipart = sign * ipart;
    f64 exp = (frac == 0) ? 0 : floor(log10l((f64)frac)) + 1;
    if (leading_zeros > 0)
        exp += leading_zeros;
    return sign * ((f64)ipart + (f64)frac * powl(10.0, -exp));
}

i64 parse_f64(f64 f, i64* ipart, i64* frac)
{
    int frac_start;
    int frac_end;
    char *buf = NULL;
    buf = snprintf_concat_float(buf, "%Lf", f);
    sscanf(buf, "%lld.%n%lld%n", ipart, &frac_start, frac, &frac_end);
    free(buf);

    int digits = (*frac == 0) ? 0 : log10l((f64)*frac);
    return frac_end - (frac_start + digits) - 1;
}

void load_f64(cpu *c, f64 f)
{
    c->r[c->dest + 2] = parse_f64(f, &c->r[c->dest], &c->r[c->dest + 1]);
}

void get_f64_operands(cpu *c, f64* f1, f64* f2)
{
    *f1 = build_f64(c->r[c->dest], c->r[c->dest + 1], c->r[c->dest + 2]);
    if (c->r[R0B] == V_FLOAT) {
        *f2 = build_f64(c->r[c->src], c->r[c->src + 1], c->r[c->src + 2]);
    } else {
        *f2 = (f64)c->r[c->src];
    }
}