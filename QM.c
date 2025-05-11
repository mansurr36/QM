#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_TERMS 50
#define VAR_COUNT 4
#define MAX_COMB 100
#define MAX_STEPS 10

typedef struct {
    int minterms[16];
    int count;
    char binary[VAR_COUNT + 1];
    bool used;
    bool is_dontcare;
} Term;

typedef struct {
    int group_count[VAR_COUNT + 1];
    Term groups[VAR_COUNT + 1][MAX_COMB];
} StepGrouping;

Term terms[MAX_TERMS];
int term_count = 0;
int prime_count = 0;
Term primes[MAX_COMB];
bool chart[MAX_COMB][MAX_TERMS];
int minterms[16], minterm_count = 0;
int dontcares[16], dontcare_count = 0;

StepGrouping steps[MAX_STEPS];
int step_total = 0;

void to_binary(int n, char* bin) {
    for (int i = VAR_COUNT - 1; i >= 0; i--) {
        bin[VAR_COUNT - 1 - i] = (n & (1 << i)) ? '1' : '0';
    }
    bin[VAR_COUNT] = '\0';
}

int count_ones(char* bin) {
    int count = 0;
    for (int i = 0; i < VAR_COUNT; i++)
        if (bin[i] == '1') count++;
    return count;
}

bool can_combine(char* a, char* b, char* res) {
    int diff = 0;
    for (int i = 0; i < VAR_COUNT; i++) {
        if (a[i] != b[i]) {
            res[i] = '-';
            diff++;
        }
        else {
            res[i] = a[i];
        }
    }
    res[VAR_COUNT] = '\0';
    return diff == 1;
}

bool exists(Term* list, int size, char* bin) {
    for (int i = 0; i < size; i++)
        if (strcmp(list[i].binary, bin) == 0)
            return true;
    return false;
}

void initial_grouping() {
    StepGrouping* init = &steps[step_total++];
    memset(init, 0, sizeof(StepGrouping));

    for (int i = 0; i < term_count; i++) {
        int ones = count_ones(terms[i].binary);
        init->groups[ones][init->group_count[ones]++] = terms[i];
    }

    printf("[초기 그룹화]\n");
    for (int g = 0; g <= VAR_COUNT; g++) {
        printf("Group %d:\n", g);
        for (int i = 0; i < init->group_count[g]; i++) {
            Term* t = &init->groups[g][i];
            printf("  ");
            for (int m = 0; m < t->count; m++) {
                printf("%2d", t->minterms[m]);
                if (m < t->count - 1) printf(",");
            }
            printf(": %s\n", t->binary);
        }
    }

    int new_count = 0;
    for (int g = 0; g <= VAR_COUNT; g++) {
        for (int i = 0; i < init->group_count[g]; i++) {
            terms[new_count++] = init->groups[g][i];
        }
    }
    term_count = new_count;
}

void generate_primes() {
    int step = 0;

    while (1) {
        StepGrouping* curr = &steps[step_total++];
        memset(curr, 0, sizeof(StepGrouping));

        for (int i = 0; i < term_count; i++) {
            int ones = count_ones(terms[i].binary);
            curr->groups[ones][curr->group_count[ones]++] = terms[i];
        }

        Term next_terms[MAX_COMB];
        int next_count = 0;
        bool combined_any = false;

        for (int g = 0; g < VAR_COUNT; g++) {
            for (int i = 0; i < curr->group_count[g]; i++) {
                for (int j = 0; j < curr->group_count[g + 1]; j++) {
                    char res[VAR_COUNT + 1];
                    if (can_combine(curr->groups[g][i].binary, curr->groups[g + 1][j].binary, res)) {
                        if (!exists(next_terms, next_count, res)) {
                            Term t;
                            strcpy(t.binary, res);
                            t.count = 0;
                            t.used = false;
                            t.is_dontcare = curr->groups[g][i].is_dontcare && curr->groups[g + 1][j].is_dontcare;

                            for (int m = 0; m < curr->groups[g][i].count; m++)
                                t.minterms[t.count++] = curr->groups[g][i].minterms[m];
                            for (int m = 0; m < curr->groups[g + 1][j].count; m++) {
                                int val = curr->groups[g + 1][j].minterms[m];
                                bool dup = false;
                                for (int n = 0; n < t.count; n++)
                                    if (t.minterms[n] == val) {
                                        dup = true;
                                        break;
                                    }
                                if (!dup) t.minterms[t.count++] = val;
                            }

                            next_terms[next_count++] = t;
                            combined_any = true;
                        }

                        curr->groups[g][i].used = true;
                        curr->groups[g + 1][j].used = true;
                    }
                }
            }
        }

        printf("\n[Step %d] Grouping\n", step + 1);
        for (int g = 0; g <= VAR_COUNT; g++) {
            printf("Group %d:\n", g);
            for (int i = 0; i < curr->group_count[g]; i++) {
                Term* t = &curr->groups[g][i];
                printf("  ");
                for (int m = 0; m < t->count; m++) {
                    printf("%2d", t->minterms[m]);
                    if (m < t->count - 1) printf(",");
                }
                printf(": %s%s\n", t->binary, t->used ? " V" : "");
            }
        }

        printf("[Step %d 결과] ", step + 1);
        for (int g = 0; g < VAR_COUNT; g++) {
            for (int i = 0; i < curr->group_count[g]; i++) {
                for (int j = 0; j < curr->group_count[g + 1]; j++) {
                    char res[VAR_COUNT + 1];
                    if (can_combine(curr->groups[g][i].binary, curr->groups[g + 1][j].binary, res)) {
                        printf("%s+%s=>%s | ",
                            curr->groups[g][i].binary,
                            curr->groups[g + 1][j].binary,
                            res);
                    }
                }
            }
        }
        printf("\n");

        for (int g = 0; g <= VAR_COUNT; g++) {
            for (int i = 0; i < curr->group_count[g]; i++) {
                Term* t = &curr->groups[g][i];
                if (!t->used && !exists(primes, prime_count, t->binary)) {
                    primes[prime_count++] = *t;
                }
            }
        }

        if (!combined_any) break;

        memcpy(terms, next_terms, sizeof(Term) * next_count);
        term_count = next_count;
        step++;
    }
}

void print_terms(const char* title, Term* list, int count) {
    printf("\n%s:\n", title);
    for (int i = 0; i < count; i++) {
        printf("%s (", list[i].binary);
        for (int j = 0; j < list[i].count; j++) {
            printf("%d", list[i].minterms[j]);
            if (j < list[i].count - 1) printf(",");
        }
        printf(")\n");
    }
}

void binary_to_expression(char* bin, char* expr) {
    char vars[] = { 'A', 'B', 'C', 'D' };
    int idx = 0;
    for (int i = 0; i < VAR_COUNT; i++) {
        if (bin[i] == '-') continue;
        expr[idx++] = vars[i];
        if (bin[i] == '0') expr[idx++] = '\'';
    }
    expr[idx] = '\0';
}

void build_chart() {
    memset(chart, 0, sizeof(chart));
    for (int i = 0; i < prime_count; i++) {
        for (int j = 0; j < minterm_count; j++) {
            bool match = true;
            char bin[VAR_COUNT + 1];
            to_binary(minterms[j], bin);
            for (int k = 0; k < VAR_COUNT; k++) {
                if (primes[i].binary[k] == '-') continue;
                if (primes[i].binary[k] != bin[k]) {
                    match = false;
                    break;
                }
            }
            if (match) chart[i][j] = true;
        }
    }
}

void print_chart() {
    printf("\n[Prime Implicant Chart]\n    ");
    for (int j = 0; j < minterm_count; j++)
        printf("%2d ", minterms[j]);
    printf("\n");

    for (int i = 0; i < prime_count; i++) {
        char expr[10];
        binary_to_expression(primes[i].binary, expr);
        printf("%-4s ", expr);
        for (int j = 0; j < minterm_count; j++) {
            if (chart[i][j])
                printf(" X ");
            else
                printf(" . ");
        }
        printf("\n");
    }
}

void find_SOP() {
    bool covered[MAX_TERMS] = { false };
    bool essential[MAX_COMB] = { false };

    for (int j = 0; j < minterm_count; j++) {
        int count = 0, idx = -1;
        for (int i = 0; i < prime_count; i++) {
            if (chart[i][j]) {
                count++;
                idx = i;
            }
        }
        if (count == 1 && idx != -1) {
            essential[idx] = true;
            for (int k = 0; k < minterm_count; k++)
                if (chart[idx][k]) covered[k] = true;
        }
    }

    while (1) {
        int max_cover = 0, best_pi = -1;
        for (int i = 0; i < prime_count; i++) {
            if (essential[i]) continue;
            int cover = 0;
            for (int j = 0; j < minterm_count; j++) {
                if (!covered[j] && chart[i][j])
                    cover++;
            }
            if (cover > max_cover) {
                max_cover = cover;
                best_pi = i;
            }
        }
        if (best_pi == -1) break;
        essential[best_pi] = true;
        for (int j = 0; j < minterm_count; j++) {
            if (chart[best_pi][j]) covered[j] = true;
        }
    }

    printf("\n최소 SOP 식: ");
    bool first = true;
    for (int i = 0; i < prime_count; i++) {
        if (essential[i]) {
            char expr[10];
            binary_to_expression(primes[i].binary, expr);
            if (!first) printf(" + ");
            printf("%s", expr);
            first = false;
        }
    }
    printf("\n");
}

int main() {
    int raw[] = { 0, 1, 2, 5, 6, 7, 8, 9, 10, 14 };
    int dc[] = { 3, 4 };
    minterm_count = sizeof(raw) / sizeof(int);
    dontcare_count = sizeof(dc) / sizeof(int);

    int all_count = 0;
    for (int i = 0; i < minterm_count; i++) {
        terms[all_count].count = 1;
        to_binary(raw[i], terms[all_count].binary);
        terms[all_count].used = false;
        terms[all_count].is_dontcare = false;
        terms[all_count].minterms[0] = raw[i];
        minterms[i] = raw[i];
        all_count++;
    }
    for (int i = 0; i < dontcare_count; i++) {
        terms[all_count].count = 1;
        to_binary(dc[i], terms[all_count].binary);
        terms[all_count].used = false;
        terms[all_count].is_dontcare = true;
        terms[all_count].minterms[0] = dc[i];
        all_count++;
    }
    term_count = all_count;

    printf("[Step 진행 요약]\n========================\n\n");
    initial_grouping();
    printf("\n========================\n");
    generate_primes();
    printf("\n========================\n");

    print_terms("Prime Implicants", primes, prime_count);
    build_chart();
    print_chart();
    find_SOP();

    return 0;
}
