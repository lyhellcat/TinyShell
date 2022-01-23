#include <math.h>
#include <stdio.h>
#include <string.h>

int n, m;
int population[100][100];
int Vaccination_Hubs_count;
int total_population = 0;

struct hubs {
    char ID[20];
    double x, y;
    int daily_vaccination_capacity;
    int total_population;
};
typedef struct hubs hub;
hub h[1010];

void get_all_and_max_population() {
    int max_pop = -1, idx_x, idx_y;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            total_population += population[i][j];
            if (population[i][j] > max_pop) {
                max_pop = population[i][j];
                idx_x = i, idx_y = j;
            }
            if (population[i][j] == max_pop) {
                double dist_new = sqrt(pow(i, 2) + pow(j, 2));
                double dist_old = sqrt(pow(idx_x, 2) + pow(idx_y, 2));
                if (dist_new < dist_old) {
                    idx_x = i;
                    idx_y = j;
                }
            }
        }
    }

    puts("Stage 1\n==========");
    printf("Total population: %d\n", total_population);
    printf("Maximum population: %d at (%02d, %02d)\n", max_pop, idx_x, idx_y);
}

void read_and_sort_the_Vaccination_Hubs() {
    int i, j;
    hub temp;
    for (i = 0; i < Vaccination_Hubs_count; i++) {
        if (strcmp(h[i].ID, h[i - 1].ID) < 0) {
            temp = h[i];
            for (j = i - 1; j >= 0 && strcmp(h[j].ID, temp.ID) > 0; j--)
                h[j + 1] = h[j];
            h[j + 1] = temp;
        }
    }
    puts("\nStage 2\n==========");
    for (int i = 0; i < Vaccination_Hubs_count; i++) {
        printf("%s %04.1f %04.1f %04d\n", h[i].ID, h[i].x, h[i].y,
               h[i].daily_vaccination_capacity);
    }
}

void find_points_far_from_vaccionation_hubs() {
    int population_far = 0;
    int cnt = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int check = 1;
            for (int k = 0; k < Vaccination_Hubs_count; k++) {
                double dist = sqrt(pow(1.0 * i - h[k].x, 2) + pow(1.0 * j - h[k].y, 2));
                if (dist <= 5) check = 0;
            }
            if (check) {
                population_far += population[i][j];
                cnt++;
            }
        }
    }
    puts("\nStage 3\n==========");
    printf("%d points (%05.2f%% of the population) are far from the hubs\n",
           cnt, 1.0 * population_far / total_population * 100);
}

char vvc[100][100][10];
void Draw_a_Vaccination_Hub_Access_Map() {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            double min_dist = 1e9;
            for (int k = 0; k < Vaccination_Hubs_count; k++) {
                double dist = sqrt(pow(1.0 * i - h[k].x, 2) + pow(1.0 * j - h[k].y, 2));
                if (dist < min_dist) {
                    min_dist = dist;
                    strcpy(vvc[i][j], h[k].ID);
                }
            }
        }
    }
    puts("\nStage 4\n==========");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (j != m - 1)
                printf("%s ", vvc[i][j]);
            else
                printf("%s\n", vvc[i][j]);
            for (int k = 0; k < Vaccination_Hubs_count; k++) {
                if (strcmp(h[k].ID, vvc[i][j]) == 0)
                    h[k].total_population += population[i][j];
            }
        }
    }
    double max_days = -1;
    char BottlenkHub[10];
    for (int i = 0; i < Vaccination_Hubs_count; i++) {
        if (1.0 * h[i].total_population / h[i].daily_vaccination_capacity >
            max_days) {
            max_days = 1.0 * h[i].total_population / h[i].daily_vaccination_capacity;
            strcpy(BottlenkHub, h[i].ID);
        }
    }

    printf(
        "\nBottleneck vaccination hub: %s\nNumber of days needed to vaccinate "
        "the population : %.0f\n",
        BottlenkHub, ceil(max_days));
}

int main() {
    scanf("%d %d", &n, &m);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            scanf("%d", &population[i][j]);
        }
    }
    while (~scanf("%s %lf %lf %d", h[Vaccination_Hubs_count].ID,
                  &h[Vaccination_Hubs_count].x, &h[Vaccination_Hubs_count].y,
                  &h[Vaccination_Hubs_count].daily_vaccination_capacity)) {
        Vaccination_Hubs_count++;
    }
    get_all_and_max_population();
    read_and_sort_the_Vaccination_Hubs();
    find_points_far_from_vaccionation_hubs();
    Draw_a_Vaccination_Hub_Access_Map();
}
