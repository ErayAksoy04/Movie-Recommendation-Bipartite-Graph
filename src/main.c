#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

// Node yapısı: Kullanıcılar ve ürünler için ortak yapı
typedef struct Node {
    int id;                // Node ID (User ID veya Item ID)
    int degree;            // Bağlantıların sayısı
    struct Edge* edges;    // Bu düğüme bağlı kenarlar
    struct Node* next;     // Bir sonraki düğüm (linked list için)
} Node;

// Edge yapısı: İki düğüm arasındaki bağlantıyı temsil eder
typedef struct Edge {
    int targetId;          // Bağlantının hedef düğümü (User -> Item veya Item -> User)
    int weight;            // Bağlantının ağırlığı (rating)
    struct Edge* next;     // Bir sonraki bağlantı
} Edge;

// Graph yapısı: Kullanıcı ve ürün kümelerini içerir
typedef struct Graph {
    Node* users;           // Kullanıcı düğümlerinin başlangıcı
    Node* items;           // Ürün düğümlerinin başlangıcı
} Graph;

// Yeni bir graph oluştur
Graph* createGraph() {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->users = NULL;
    graph->items = NULL;
    return graph;
}

// Yeni bir düğüm oluştur
Node* createNode(int id) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->id = id;
    node->degree = 0;
    node->edges = NULL;
    node->next = NULL;
    return node;
}

// Graph'e yeni bir kullanıcı ekle
void addUser(Graph* graph, int userId) {
    // Kullanıcının graph'ta zaten olup olmadığını kontrol et
    Node* current = graph->users;
    while (current) {
        if (current->id == userId) return; // Kullanıcı zaten varsa ekleme
        current = current->next;
    }

    // Kullanıcı yoksa yeni bir düğüm oluştur ve ekle
    Node* user = createNode(userId);
    user->next = graph->users;
    graph->users = user;
}

// Graph'e yeni bir ürün ekle
void addItem(Graph* graph, int itemId) {
    // Ürünün graph'ta zaten olup olmadığını kontrol et
    Node* current = graph->items;
    while (current) {
        if (current->id == itemId) return; // Ürün zaten varsa ekleme
        current = current->next;
    }

    // Ürün yoksa yeni bir düğüm oluştur ve ekle
    Node* item = createNode(itemId);
    item->next = graph->items;
    graph->items = item;
}

// Kullanıcı ve ürün arasında bir bağlantı oluştur
void addEdge(Graph* graph, int userId, int itemId, int weight) {
    Node* user = graph->users;
    Node* item = graph->items;

    // Kullanıcı düğümünü bul
    while (user && user->id != userId) user = user->next;

    // Ürün düğümünü bul
    while (item && item->id != itemId) item = item->next;

    // Bağlantılar ekleniyor
    if (user && item) {
        Edge* newEdgeUser = (Edge*)malloc(sizeof(Edge));
        newEdgeUser->targetId = itemId;
        newEdgeUser->weight = weight;
        newEdgeUser->next = user->edges;
        user->edges = newEdgeUser;
        user->degree++;

        Edge* newEdgeItem = (Edge*)malloc(sizeof(Edge));
        newEdgeItem->targetId = userId;
        newEdgeItem->weight = weight;
        newEdgeItem->next = item->edges;
        item->edges = newEdgeItem;
        item->degree++;
    }
}

void loadGraphFromFile(Graph* graph, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Dosya acilamadi");
        return;
    }

    int userId, itemId, rating, timestamp; // timestamp'i okuyacağız ama kullanmayacağız
    while (fscanf(file, "%d\t%d\t%d\t%d", &userId, &itemId, &rating, &timestamp) != EOF) {
        // Kullanıcı ve ürün ID'lerini kontrol et
        if (userId < 1 || userId > 943) {
            printf("Hatali User ID: %d (atlandi)\n", userId);
            continue;
        }
        if (itemId < 1 || itemId > 1682) {
            printf("Hatali Item ID: %d (atlandi)\n", itemId);
            continue;
        }

        // Veriyi ekrana yazdır (hata ayıklama için)
        printf("Reading: User %d, Item %d, Rating %d\n", userId, itemId, rating);

        // Kullanıcı ve ürün düğümlerini ekle
        addUser(graph, userId);
        addItem(graph, itemId);

        // Kullanıcı ve ürün arasına bağlantı ekle
        addEdge(graph, userId, itemId, rating);
    }

    fclose(file);
}


// Kullanıcıları ve bağlantılarını yazdır
void printUsers(Graph* graph) {
    printf("Users and their connections:\n");
    Node* user = graph->users;
    while (user != NULL) {
        printf("User %d (Degree: %d): ", user->id, user->degree);
        Edge* edge = user->edges;
        while (edge != NULL) {
            printf("-> Item %d (Weight: %d) ", edge->targetId, edge->weight);
            edge = edge->next;
        }
        printf("\n");
        user = user->next;
    }
}

// Ürünleri ve bağlantılarını yazdır
void printItems(Graph* graph) {
    printf("Items and their connections:\n");
    Node* item = graph->items;
    while (item != NULL) {
        printf("Item %d (Degree: %d): ", item->id, item->degree);
        Edge* edge = item->edges;
        while (edge != NULL) {
            printf("-> User %d (Weight: %d) ", edge->targetId, edge->weight);
            edge = edge->next;
        }
        printf("\n");
        item = item->next;
    }
}

// Kullanıcının izlemediği filmlerden rastgele birini öner
int recommendRandom(Graph* graph, int userId) {
    Node* user = graph->users;
    while (user && user->id != userId) {
        user = user->next;
    }

    if (!user) {
        printf("User %d bulunamadi.\n", userId);
        return -1;
    }

    printf("User %d bulundu, baglanti kontrol ediliyor.\n", userId);

    // Kullanıcının bağlantılı olduğu tüm filmleri bir listeye ekle
    int watched[1682] = {0}; // 1682, toplam film sayısı
    Edge* edge = user->edges;
    while (edge) {
        watched[edge->targetId - 1] = 1; // İzlenen filmleri işaretle
        edge = edge->next;
    }

    // İzlenmemiş filmleri listele
    int unwatched[1682], count = 0;
    Node* item = graph->items;
    while (item) {
        if (!watched[item->id - 1]) {
            unwatched[count++] = item->id;
        }
        item = item->next;
    }

    printf("Unwatched movies for User %d: %d\n", userId, count);

    if (count == 0) {
        printf("User %d icin izlenmemis film yok.\n", userId);
        return -1;
    }

    // Rastgele bir film seç
    srand(time(NULL));
    int randomIndex = rand() % count;
    return unwatched[randomIndex];
}

void printAllUsers(Graph* graph) {
    Node* user = graph->users;
    printf("All Users:\n");
    while (user) {
        printf("User ID: %d\n", user->id);
        user = user->next;
    }
}

void printUserConnections(Graph* graph, int userId) {
    Node* user = graph->users;
    while (user && user->id != userId) {
        user = user->next;
    }

    if (!user) {
        printf("User %d bulunamadi.\n", userId);
        return;
    }

    printf("User %d connections:\n", user->id);
    Edge* edge = user->edges;
    while (edge) {
        printf("-> Item %d (Weight: %d)\n", edge->targetId, edge->weight);
        edge = edge->next;
    }
}

int recommendByDegree(Graph* graph, int userId) {
    Node* user = graph->users;
    while (user && user->id != userId) {
        user = user->next;
    }

    if (!user) {
        printf("User %d bulunamadi.\n", userId);
        return -1;
    }

    printf("User %d bulundu, izlenmemis filmler kontrol ediliyor.\n", userId);

    // Kullanıcının bağlantılı olduğu tüm filmleri bir listeye ekle
    int watched[1682] = {0}; // İzlenen filmleri işaretle
    Edge* edge = user->edges;
    while (edge) {
        watched[edge->targetId - 1] = 1;
        edge = edge->next;
    }

    // İzlenmemiş filmleri ve derecelerini bul
    Node* item = graph->items;
    int maxDegree = -1;
    int unwatched[1682], count = 0;
    while (item) {
        if (!watched[item->id - 1]) {
            if (item->degree > maxDegree) {
                // Yeni en yüksek derece bulundu, listeyi sıfırla
                maxDegree = item->degree;
                count = 0;
                unwatched[count++] = item->id;
            } else if (item->degree == maxDegree) {
                // Aynı dereceye sahip başka bir film
                unwatched[count++] = item->id;
            }
        }
        item = item->next;
    }

    if (count == 0) {
        printf("User %d icin izlenmemis film yok.\n", userId);
        return -1;
    }

    // Aynı dereceye sahip filmlerden rastgele birini seç
    srand(time(NULL));
    int randomIndex = rand() % count;
    printf("User %d icin en yuksek dereceye sahip film: %d (Degree: %d)\n",
           userId, unwatched[randomIndex], maxDegree);
    return unwatched[randomIndex];
}

int recommendByCommonUser(Graph* graph, int userId) {
    Node* user = graph->users;
    while (user && user->id != userId) {
        user = user->next;
    }

    if (!user) {
        printf("User %d bulunamadi.\n", userId);
        return -1;
    }

    printf("User %d bulundu, ortak kullanicilar kontrol ediliyor.\n", userId);

    // Kullanıcının izlediği filmleri işaretle
    int watched[1682] = {0}; // İzlenen filmleri işaretle
    Edge* edge = user->edges;
    while (edge) {
        watched[edge->targetId - 1] = 1;
        edge = edge->next;
    }

    // Tüm kullanıcılarla ortak film sayısını hesapla
    Node* otherUser = graph->users;
    int maxCommon = 0;
    Node* commonUser = NULL;

    while (otherUser) {
        if (otherUser->id == userId) {
            otherUser = otherUser->next;
            continue; // Kendisiyle karşılaştırma yapma
        }

        int commonCount = 0;
        Edge* otherEdge = otherUser->edges;
        while (otherEdge) {
            if (watched[otherEdge->targetId - 1]) {
                commonCount++; // Ortak izleme bulundu
            }
            otherEdge = otherEdge->next;
        }

        if (commonCount > maxCommon) {
            maxCommon = commonCount;
            commonUser = otherUser;
        }

        otherUser = otherUser->next;
    }

    if (!commonUser) {
        printf("User %d ile ortak film izleyen kullanici bulunamadi.\n", userId);
        return -1;
    }

    printf("User %d icin en fazla ortak film izleyen: User %d (Common: %d)\n",
           userId, commonUser->id, maxCommon);

    // Ortak kullanıcının yüksek puan verdiği ve izlenmemiş filmleri ara
    Edge* commonEdge = commonUser->edges;
    int maxRating = 0;
    int recommendedMovie = -1;

    while (commonEdge) {
        int movieId = commonEdge->targetId;
        int rating = commonEdge->weight;

        if (!watched[movieId - 1] && rating > maxRating) {
            maxRating = rating;
            recommendedMovie = movieId;
        }

        commonEdge = commonEdge->next;
    }

    if (recommendedMovie == -1) {
        printf("User %d icin izlenmemis uygun film bulunamadi.\n", userId);
    } else {
        printf("User %d icin ortak kullanicidan onerilen film: %d (Rating: %d)\n",
               userId, recommendedMovie, maxRating);
    }

    return recommendedMovie;
}

int bfsWeightedDistance(Graph* graph, int userId, int itemId) {
    int queue[10000], front = 0, rear = 0;
    int visited[1682] = {0}; // Kullanıcılar ve filmler için ziyaret dizisi
    int distance[1682]; // Mesafe dizisi
    for (int i = 0; i < 1682; i++) distance[i] = INT_MAX; // Başlangıçta tüm mesafeler sonsuz

    queue[rear++] = userId;
    visited[userId - 1] = 1;
    distance[userId - 1] = 0;

    while (front < rear) {
        int currentUser = queue[front++];
        Node* userNode = graph->users;

        while (userNode && userNode->id != currentUser) {
            userNode = userNode->next;
        }

        if (!userNode || !userNode->edges) {
            continue; // Kullanıcı veya bağlantısı olmayan düğüm
        }

        Edge* edge = userNode->edges;
        while (edge) {
            int targetId = edge->targetId;

            // Eğer hedef filme ulaşırsak, mesafeyi döndür
            if (targetId == itemId) {
                return distance[currentUser - 1] + edge->weight;
            }

            // Kullanıcıları sıraya ekle
            if (!visited[targetId - 1] && targetId <= 943) { // Kullanıcı aralığında
                visited[targetId - 1] = 1;
                queue[rear++] = targetId;
                distance[targetId - 1] = distance[currentUser - 1] + edge->weight;
            }

            edge = edge->next;
        }
    }

    return INT_MAX; // Eğer hedef filme ulaşılamazsa
}

int recommendByWeightedDistance(Graph* graph, int userId) {
    Node* user = graph->users;
    while (user && user->id != userId) {
        user = user->next;
    }

    if (!user) {
        printf("User %d bulunamadi.\n", userId);
        return -1;
    }

    printf("User %d bulundu, agirlikli mesafeler kontrol ediliyor.\n", userId);

    int watched[1682] = {0}; // Kullanıcının izlediği filmleri işaretle
    Edge* edge = user->edges;
    while (edge) {
        watched[edge->targetId - 1] = 1;
        edge = edge->next;
    }

    int minDistance = INT_MAX;
    int recommendedMovie = -1;
    int unwatched[1682], count = 0;

    Node* item = graph->items;
    while (item) {
        if (!watched[item->id - 1]) { // Sadece izlenmemiş filmleri kontrol et
            int distance = bfsWeightedDistance(graph, userId, item->id);

            if (distance == INT_MAX) { // Eğer filme ulaşılamıyorsa atla
                item = item->next;
                continue;
            }

            if (distance == minDistance) {
                unwatched[count++] = item->id; // Aynı mesafede olan filmleri ekle
            } else if (distance < minDistance) {
                minDistance = distance;
                count = 0;
                unwatched[count++] = item->id; // Yeni minimum mesafeyi ekle
            }
        }
        item = item->next;
    }

    if (count > 0) {
        srand(time(NULL));
        recommendedMovie = unwatched[rand() % count]; // Rastgele birini seç
    }

    if (recommendedMovie == -1) {
        printf("User %d icin izlenmemis uygun film bulunamadi.\n", userId);
    } else {
        printf("User %d icin en yakin agirlikli mesafeye sahip film: %d (Distance: %d)\n",
               userId, recommendedMovie, minDistance);
    }

    return recommendedMovie;
}

int recommendImprovedDynamic(Graph* graph, int userId) {
    Node* user = graph->users;
    while (user && user->id != userId) {
        user = user->next;
    }

    if (!user) {
        printf("User %d bulunamadi.\n", userId);
        return -1;
    }

    printf("User %d bulundu, dinamik kisiye ozel filmler kontrol ediliyor.\n", userId);

    int watched[1682] = {0}; // Kullanıcının izlediği filmleri işaretle
    Edge* edge = user->edges;
    while (edge) {
        watched[edge->targetId - 1] = 1;
        edge = edge->next;
    }

    // Her film için skorları hesaplamak
    double filmScores[1682] = {0};
    int recommendedMovie = -1;
    double maxScore = -1;

    Node* item = graph->items;
    while (item) {
        if (!watched[item->id - 1]) { // İzlenmemiş filmleri kontrol et
            int similarUsers = 0;
            int totalRatings = 0;
            int lowRatings = 0;

            Edge* itemEdge = item->edges;
            while (itemEdge) {
                totalRatings++;
                if (itemEdge->weight <= 2) { // Düşük puanları say
                    lowRatings++;
                }
                if (watched[itemEdge->targetId - 1]) {
                    similarUsers += itemEdge->weight; // Benzer kullanıcıların puanları
                }
                itemEdge = itemEdge->next;
            }

            if (totalRatings == 0 || lowRatings > totalRatings / 2) {
                item = item->next;
                continue; // Düşük puan ağırlıklı filmleri atla
            }

            // Skor hesaplama
            double score = ((2.0 * similarUsers) + (0.5 * item->degree)) / totalRatings;
            filmScores[item->id - 1] = score;

            if (score > maxScore) {
                maxScore = score;
                recommendedMovie = item->id;
            }
        }
        item = item->next;
    }

    if (recommendedMovie == -1) {
        printf("User %d icin dinamik kisiye ozel uygun film bulunamadi.\n", userId);
    } else {
        printf("User %d icin dinamik kisiye ozel onerilen film: %d (Score: %.2f)\n",
               userId, recommendedMovie, maxScore);
    }

    return recommendedMovie;
}

int main() {
    // Graph olustur ve veri yukle
    Graph* graph = createGraph();
    loadGraphFromFile(graph, "u.data");
    printf("Graph basariyla olusturuldu.\n");

    // Test edilecek kullanici ID'si
    int testUserId = 5; // Ornek olarak User 5

    // 1. Algoritma: Rastgele Film Onerisi
    printf("\nAlgoritma 1: Rastgele Film Onerisi\n");
    int randomMovie = recommendRandom(graph, testUserId);
    if (randomMovie != -1) {
        printf("User %d icin rastgele onerilen film: %d\n", testUserId, randomMovie);
    } else {
        printf("User %d icin rastgele uygun film bulunamadi.\n", testUserId);
    }

    // 2. Algoritma: En Yuksek Dereceli Film Onerisi
    printf("\nAlgoritma 2: En Yuksek Dereceli Film Onerisi\n");
    int highestDegreeMovie = recommendByDegree(graph, testUserId);
    if (highestDegreeMovie != -1) {
        printf("User %d icin en yuksek dereceye sahip film: %d\n", testUserId, highestDegreeMovie);
    } else {
        printf("User %d icin en yuksek dereceye sahip uygun film bulunamadi.\n", testUserId);
    }

    // 3. Algoritma: Ortak Kullaniciya Gore Film Onerisi
    printf("\nAlgoritma 3: Ortak Kullaniciya Gore Film Onerisi\n");
    int similarUserMovie = recommendByCommonUser(graph, testUserId);
    if (similarUserMovie != -1) {
        printf("User %d icin ortak kullaniciya dayali onerilen film: %d\n", testUserId, similarUserMovie);
    } else {
        printf("User %d icin ortak kullaniciya dayali uygun film bulunamadi.\n", testUserId);
    }

    // 4. Algoritma: Agirlikli Mesafeye Gore Film Onerisi
    printf("\nAlgoritma 4: Agirlikli Mesafeye Gore Film Onerisi\n");
    int weightedDistanceMovie = recommendByWeightedDistance(graph, testUserId);
    if (weightedDistanceMovie != -1) {
        printf("User %d icin agirlikli mesafeye dayali onerilen film: %d\n", testUserId, weightedDistanceMovie);
    } else {
        printf("User %d icin agirlikli mesafeye dayali uygun film bulunamadi.\n", testUserId);
    }

    // 5. Algoritma: Gelistirilmis Dinamik Kisiye Ozel Film Onerisi
    printf("\nAlgoritma 5: Gelistirilmis Dinamik Kisiye Ozel Film Onerisi\n");
    int improvedDynamicMovie = recommendImprovedDynamic(graph, testUserId);
    if (improvedDynamicMovie != -1) {
        printf("User %d icin dinamik kisiye ozel onerilen film: %d\n", testUserId, improvedDynamicMovie);
    } else {
        printf("User %d icin dinamik kisiye ozel uygun film bulunamadi.\n", testUserId);
    }

    return 0;
}