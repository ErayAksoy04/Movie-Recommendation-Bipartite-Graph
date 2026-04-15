#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

//DEBUG Modunu aktifleştirmek aşağıdaki kodu yorum satırı yapmaktan çıkarın

//#define DEBUG

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
            // Hatalı kullanıcı ID'sini atla
            continue;
        }
        if (itemId < 1 || itemId > 1682) {
            // Hatalı ürün ID'sini atla
            continue;
        }

        // Eğer DEBUG modu aktifse, veriyi ekrana yazdır
#ifdef DEBUG
        printf("Reading: User %d, Item %d, Rating %d\n", userId, itemId, rating);
#endif

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

    printf("User %d icin izlenmemis film sayisi: %d\n", userId, count);

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
    printf("User %d icin en yuksek dereceye sahip film: %d (Derece: %d)\n",
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
        printf("User %d icin en yakin agirlikli mesafeye sahip film: %d (Mesafe: %d)\n",
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

int recommendByRandomWalk(Graph* graph, int userId, int maxSteps, double restartProbability) {
    Node* user = graph->users;
    while (user && user->id != userId) {
        user = user->next;
    }

    if (!user) {
        printf("User %d bulunamadi.\n", userId);
        return -1;
    }

    printf("User %d bulundu, rastgele yuruyus baslatiliyor.\n", userId);

    // Ziyaret sayısı tutucu
    int visits[1682] = {0}; // Toplam 1682 film
    srand(time(NULL)); // Rastgelelik için zaman tabanlı tohum

    Node* current = user; // Yürüyüşe başlangıç noktası
    for (int step = 0; step < maxSteps; step++) {
        if (!current || !current->edges) {
            // Eğer çıkmaz bir noktaya ulaşıldıysa, yeniden başlat
            current = user;
            continue;
        }

        // Rastgele yeniden başlama
        if ((rand() / (double)RAND_MAX) < restartProbability) {
            current = user;
            continue;
        }

        // Rastgele bir bağlantı seç
        int edgeCount = 0;
        Edge* edge = current->edges;
        while (edge) {
            edgeCount++;
            edge = edge->next;
        }

        int randomEdge = rand() % edgeCount; // Rastgele kenar seçimi
        edge = current->edges;
        while (randomEdge-- > 0) {
            edge = edge->next;
        }

        if (edge->targetId > 943) { // Eğer hedef bir filmse
            visits[edge->targetId - 1]++; // Ziyareti artır
        }

        // Bir sonraki düğüme geç
        current = (edge->targetId <= 943) ? graph->users : graph->items;
        while (current && current->id != edge->targetId) {
            current = current->next;
        }
    }

    // En çok ziyaret edilen filmi bul
    int maxVisits = 0;
    int recommendedMovie = -1;
    for (int i = 0; i < 1682; i++) {
        if (visits[i] > maxVisits) {
            maxVisits = visits[i];
            recommendedMovie = i + 1;
        }
    }

    if (recommendedMovie == -1) {
        printf("User %d icin rastgele yuruyuse dayali uygun film bulunamadi.\n", userId);
    } else {
        printf("User %d icin rastgele yuruyuse dayali onerilen film: %d (Visits: %d)\n",
               userId, recommendedMovie, maxVisits);
    }

    return recommendedMovie;
}

void displayMenu() {
    printf("\n===== Film Oneri Sistemi =====\n");
    printf("1. Kullanici ID'yi gir\n");
    printf("2. Kullanici ID degistir\n");
    printf("3. Rastgele film oner\n");
    printf("4. En yuksek dereceli filmi oner\n");
    printf("5. Ortak kullaniciya gore film oner\n");
    printf("6. Agirlikli mesafeye gore film oner\n");
    printf("7. Dinamik kullaniciya ozel film oner\n");
    printf("8. Rastgele yuruyus mesafesi bazli film oner\n");
    printf("9. Tum algoritmalari calistir ve sonuclari goster\n");
    printf("10. Help (Algoritmalar hakkinda bilgi al)\n");
    printf("11. Performans olcumu yap\n");
    printf("12. Model basari olcumu yap\n");
    printf("13. Cikis\n");
    printf("==============================\n");
}

void helpMenu() {
    printf("\n===== Algoritmalar Hakkinda Bilgi =====\n");
    printf("1. Rastgele film oneri algoritmasi, hicbir kriter gozetmeden rastgele bir film onerir.\n");
    printf("2. En yuksek dereceli film algoritmasi, izlenmemis filmler arasindan en populer olani secer.\n");
    printf("3. Ortak kullanici algoritmasi, en cok ortak film izlenen kullanicidan film onerir.\n");
    printf("4. Agirlikli mesafeye gore algoritma, en yakin mesafedeki filmi onerir.\n");
    printf("5. Dinamik algoritma, kisiye ozel dinamik bir model ile film onerir.\n");
    printf("6. Rastgele yuruyus algoritmasi, graf uzerinde rastgele hareket ederek film onerir.\n");
    printf("=======================================\n");
}

void measurePerformance(Graph* graph, int userId, int maxSteps, double restartProbability) {
    printf("\nPerformans olcumu basliyor...\n");
    clock_t start, end;
    double duration;

    printf("\n--- Algoritma Performanslari ---\n");

    // Algoritma 1: Rastgele Film Oneri
    start = clock();
    recommendRandom(graph, userId);
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Algoritma 1: Rastgele Film Oneri - Sure: %.4f saniye\n", duration);

    // Algoritma 2: En Yuksek Dereceli Film Oneri
    start = clock();
    recommendByDegree(graph, userId);
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Algoritma 2: En Yuksek Dereceli Film Oneri - Sure: %.4f saniye\n", duration);

    // Algoritma 3: Ortak Kullaniciya Gore Film Oneri
    start = clock();
    recommendByCommonUser(graph, userId);
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Algoritma 3: Ortak Kullaniciya Gore Film Oneri - Sure: %.4f saniye\n", duration);

    // Algoritma 4: Agirlikli Mesafeye Gore Film Oneri
    start = clock();
    recommendByWeightedDistance(graph, userId);
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Algoritma 4: Agirlikli Mesafeye Gore Film Oneri - Sure: %.4f saniye\n", duration);

    // Algoritma 5: Dinamik Kullaniciya Ozel Film Oneri
    start = clock();
    recommendImprovedDynamic(graph, userId);
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Algoritma 5: Dinamik Kullaniciya Ozel Film Oneri - Sure: %.4f saniye\n", duration);

    // Algoritma 6: Rastgele Yuruyus Mesafesi Bazli Film Oneri
    start = clock();
    recommendByRandomWalk(graph, userId, maxSteps, restartProbability);
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Algoritma 6: Rastgele Yuruyus Mesafesi Bazli Film Oneri - Sure: %.4f saniye\n", duration);

    printf("--- Performans olcumu tamamlandi ---\n\n");
}

// Her film için ortalama puanı hesapla
void calculateAverageRatings(Graph* graph, double* averageRatings) {
    for (int i = 0; i < 1682; i++) {
        averageRatings[i] = 0.0;
    }

    Node* item = graph->items;
    while (item) {
        double totalRating = 0.0;
        int ratingCount = 0;

        Edge* edge = item->edges;
        while (edge) {
            totalRating += edge->weight;
            ratingCount++;
            edge = edge->next;
        }

        if (ratingCount > 0) {
            averageRatings[item->id - 1] = totalRating / ratingCount;
        }

        item = item->next;
    }
}

// Precision, Recall ve F1-Score hesaplama
void calculateMetricsBasedOnAverage(double recommendedMovieRating, double threshold, double* precision, double* recall, double* f1Score) {
    int isAboveThreshold = (recommendedMovieRating >= threshold) ? 1 : 0;

    *precision = isAboveThreshold ? 1.0 : 0.0;
    *recall = isAboveThreshold ? 1.0 : 0.0; // Tek bir öneri için recall da precision ile aynı
    *f1Score = (*precision > 0 && *recall > 0) ? (2 * (*precision) * (*recall)) / ((*precision) + (*recall)) : 0.0;
}

// Model başarısı ölçümü
void measureModelPerformanceWithRatings(Graph* graph, double threshold) {
    double averageRatings[1682];
    calculateAverageRatings(graph, averageRatings);

    printf("\n--- Model Basarisi Olcumu (Ortalama Puanlara Dayali) ---\n");

    double precision, recall, f1Score;
    int recommendedMovie;

    // Algoritma 1: Rastgele Film Oneri
    recommendedMovie = recommendRandom(graph, 243); // Örnek UserID
    calculateMetricsBasedOnAverage(averageRatings[recommendedMovie - 1], threshold, &precision, &recall, &f1Score);
    printf("Algoritma 1: Rastgele Film Oneri - Precision: %.2f, Recall: %.2f, F1-Score: %.2f\n", precision, recall, f1Score);

    // Algoritma 2: En Yuksek Dereceli Film Oneri
    recommendedMovie = recommendByDegree(graph, 243);
    calculateMetricsBasedOnAverage(averageRatings[recommendedMovie - 1], threshold, &precision, &recall, &f1Score);
    printf("Algoritma 2: En Yuksek Dereceli Film Oneri - Precision: %.2f, Recall: %.2f, F1-Score: %.2f\n", precision, recall, f1Score);

    // Algoritma 3: Ortak Kullaniciya Gore Film Oneri
    recommendedMovie = recommendByCommonUser(graph, 243);
    calculateMetricsBasedOnAverage(averageRatings[recommendedMovie - 1], threshold, &precision, &recall, &f1Score);
    printf("Algoritma 3: Ortak Kullaniciya Gore Film Oneri - Precision: %.2f, Recall: %.2f, F1-Score: %.2f\n", precision, recall, f1Score);

    // Algoritma 4: Agirlikli Mesafeye Gore Film Oneri
    recommendedMovie = recommendByWeightedDistance(graph, 243);
    calculateMetricsBasedOnAverage(averageRatings[recommendedMovie - 1], threshold, &precision, &recall, &f1Score);
    printf("Algoritma 4: Agirlikli Mesafeye Gore Film Oneri - Precision: %.2f, Recall: %.2f, F1-Score: %.2f\n", precision, recall, f1Score);

    // Algoritma 5: Dinamik Kullaniciya Ozel Film Oneri
    recommendedMovie = recommendImprovedDynamic(graph, 243);
    calculateMetricsBasedOnAverage(averageRatings[recommendedMovie - 1], threshold, &precision, &recall, &f1Score);
    printf("Algoritma 5: Dinamik Kullaniciya Ozel Film Oneri - Precision: %.2f, Recall: %.2f, F1-Score: %.2f\n", precision, recall, f1Score);

    // Algoritma 6: Rastgele Yuruyus Mesafesi Bazli Film Oneri
    recommendedMovie = recommendByRandomWalk(graph, 243, 200, 0.15); // maxSteps = 200, restartProbability = 0.15
    calculateMetricsBasedOnAverage(averageRatings[recommendedMovie - 1], threshold, &precision, &recall, &f1Score);
    printf("Algoritma 6: Rastgele Yuruyus Mesafesi Bazli Film Oneri - Precision: %.2f, Recall: %.2f, F1-Score: %.2f\n", precision, recall, f1Score);

    printf("--- Model Basarisi Olcumu Tamamlandi ---\n");
}


int main() {
    // Graph olustur ve veri yukle
    Graph* graph = createGraph();
    loadGraphFromFile(graph, "u.data");
    printf("Graph basariyla olusturuldu.\n");

    int userId = -1; // Kullanici ID baslangicta gecersiz
    int choice;
    int maxSteps = 200; // Rastgele yuruyus icin adim sayisi
    double restartProbability = 0.15; // Rastgele yeniden baslama olasiligi

    while (1) {
        displayMenu();
        printf("Bir secenek girin: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Lutfen bir kullanici ID girin: ");
                scanf("%d", &userId);
                printf("UserID %d olarak ayarlandi.\n", userId);
                break;

            case 2:
                printf("Kullanici ID'yi degistir: ");
                scanf("%d", &userId);
                printf("Yeni UserID: %d\n", userId);
                break;

            case 3:
                if (userId == -1) {
                    printf("Lutfen once bir kullanici ID girin (1. secenek).\n");
                } else {
                    int movie = recommendRandom(graph, userId);
                    if (movie != -1) {
                        printf("User %d icin rastgele onerilen film: %d\n", userId, movie);
                    } else {
                        printf("User %d icin uygun film bulunamadi.\n", userId);
                    }
                }
                break;

            case 4:
                if (userId == -1) {
                    printf("Lutfen once bir kullanici ID girin (1. secenek).\n");
                } else {
                    int movie = recommendByDegree(graph, userId);
                    if (movie != -1) {
                        printf("User %d icin en yuksek dereceli film: %d\n", userId, movie);
                    } else {
                        printf("User %d icin uygun film bulunamadi.\n", userId);
                    }
                }
                break;

            case 5:
                if (userId == -1) {
                    printf("Lutfen once bir kullanici ID girin (1. secenek).\n");
                } else {
                    int movie = recommendByCommonUser(graph, userId);
                    if (movie != -1) {
                        printf("User %d icin ortak kullaniciya gore onerilen film: %d\n", userId, movie);
                    } else {
                        printf("User %d icin uygun film bulunamadi.\n", userId);
                    }
                }
                break;

            case 6:
                if (userId == -1) {
                    printf("Lutfen once bir kullanici ID girin (1. secenek).\n");
                } else {
                    int movie = recommendByWeightedDistance(graph, userId);
                    if (movie != -1) {
                        printf("User %d icin agirlikli mesafeye gore onerilen film: %d\n", userId, movie);
                    } else {
                        printf("User %d icin uygun film bulunamadi.\n", userId);
                    }
                }
                break;

            case 7:
                if (userId == -1) {
                    printf("Lutfen once bir kullanici ID girin (1. secenek).\n");
                } else {
                    int movie = recommendImprovedDynamic(graph, userId);
                    if (movie != -1) {
                        printf("User %d icin dinamik kisiye ozel onerilen film: %d\n", userId, movie);
                    } else {
                        printf("User %d icin uygun film bulunamadi.\n", userId);
                    }
                }
                break;

            case 8:
                if (userId == -1) {
                    printf("Lutfen once bir kullanici ID girin (1. secenek).\n");
                } else {
                    int movie = recommendByRandomWalk(graph, userId, maxSteps, restartProbability);
                    if (movie != -1) {
                        printf("User %d icin rastgele yuruyus mesafesi bazli onerilen film: %d\n", userId, movie);
                    } else {
                        printf("User %d icin uygun film bulunamadi.\n", userId);
                    }
                }
                break;

            case 9:
                if (userId == -1) {
                    printf("Lutfen once bir kullanici ID girin (1. secenek).\n");
                } else {
                    printf("Tum algoritmalar calistiriliyor...\n");
                    recommendRandom(graph, userId);
                    recommendByDegree(graph, userId);
                    recommendByCommonUser(graph, userId);
                    recommendByWeightedDistance(graph, userId);
                    recommendImprovedDynamic(graph, userId);
                    recommendByRandomWalk(graph, userId, maxSteps, restartProbability);
                }
                break;

            case 10:
                helpMenu();
                break;

            case 11:
                measurePerformance(graph, userId, maxSteps, restartProbability);
                break;

            case 12:
                printf("Lutfen threshold degerini giriniz (ornek: 3.5): ");
                double threshold;
                scanf("%lf", &threshold);
                measureModelPerformanceWithRatings(graph, threshold);
                break;

            case 13:
                printf("Program sonlandiriliyor. Iyi gunler!\n");
                return 0;

            default:
                printf("Gecersiz bir secenek girdiniz, lutfen tekrar deneyin.\n");
        }
    }
    return 0;
}