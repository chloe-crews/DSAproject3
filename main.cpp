#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>
#include <unordered_set>
using namespace std;
struct Artist {
    string id;
    unordered_map<string, string> songs;//name to ID
    Artist() : id() {};
    explicit Artist(string id) : id(std::move(id)) {};
    Artist(string id, const string& songName, const string& songId) : id(std::move(id))  {
        songs.emplace(songName, songId);
    }
};
struct songAttributes {//easy to use as a parameter
  double danceability;
  double energy;
  double key;
  double loudness;
  double mode;
  double speechiness;
  double acousticness;
  double instrumentalness;
  double liveliness;
  double valence;
  double tempo;
};
struct Song {
  /* Vector Indices Matched to Corresponding Variables */
  // vector<string> info;
  string id;
  string name;
  string album;
  string album_id;
  vector<string> artists;
  vector<string> artist_ids;
  // id, name, album, album_id, artists, artist_ids
  //  0    1     2        3        4          5

  bool explicit_song = false;  // whether the song is explicit

  songAttributes attributes{};
  // vector<double> attributes;
  // danceability, energy, key, loudness, mode, speechiness, acousticness,
  // instrumentalness, liveliness, valence, tempo
  //      0           1     2       3       4        5             6 7 8 9 10
  int time_signature = 0;
  int year = 0;
};

struct compareSongPairs {
  bool operator()(pair<string, double>& p1, pair<string, double>& p2) {
    return p1.second < p2.second;
  }
};
struct compareSongPairsGreater {
  bool operator()(pair<string, double>& p1, pair<string, double>& p2) {
    return p1.second > p2.second;//we need a min heap
  }
};
void removeDoubleDoubleQuotes(string& str) {
    for(int i = 0; i < str.size() - 1; i++) {
        if(str[i] == '"' && str[i+1] == '"') {
            str.erase(i+1, 1);
        }
    }
}
std::string getField(istringstream& buffer) {
    if (buffer.eof()) return "";
    if (buffer.peek() == '\"') {//if next is a ""
        buffer.ignore();//
        streampos beginning = buffer.tellg();//https://cplusplus.com/reference/istream/istream/tellg/
        int length = 0;
        while (true) {
            int curr = buffer.get();
            if (curr == '"' && buffer.peek() == '"') {//if "" we ignore
                buffer.ignore();
                ++length;
            } else if (curr == EOF || curr == '"' && buffer.peek() == ',') {
                break;
            }
            ++length;
        }
        buffer.seekg(beginning);//go back to the beginning
        string ans(length, ' ');//https://codereview.stackexchange.com/questions/28727/reading-n-chars-from-stream-to-string
        buffer.read(&ans[0], length);
        removeDoubleDoubleQuotes(ans);
        buffer.ignore();
        buffer.ignore();
        return ans;
    } else {
        string ans;
        getline(buffer, ans, ',');
        return ans;
    }
}
string getListFromBuffer(istringstream& list) {
    int open = 1;
    int length = 0;
    list.ignore();
    streampos begin = list.tellg();
    for(;;) {
        int curr = list.get();
        if(curr == EOF) {
            break;
        }
        if(curr == '[') {
            open++;
        }
        if(curr == ']') {
            open--;
            if(open == 0) {
                ++length;
                break;
            }
        }
        ++length;
    }
    list.seekg(begin);
    string returner(length, ' ');
    list.get(&returner[0], length);
    list.ignore();
    return returner;

}
vector<string> splitInternalLists(istringstream& buffer) {
    if(buffer.peek() == '[') {
        string artist = getListFromBuffer(buffer);
        artist.erase(artist.size()-1, 1);
        buffer.ignore();
        artist.erase(0,1);
        artist.erase(artist.size()-1,1);
        removeDoubleDoubleQuotes(artist);
        return {artist};
    } else {
        buffer.ignore();
        string list = getListFromBuffer(buffer);
        list.erase(list.size()-1, 1);
        buffer.ignore();
        buffer.ignore();
        istringstream listSS(list);
        vector<string> ans;
        while(!listSS.eof()) {
            if(listSS.peek() == '\'') {
                listSS.ignore();
                streampos begin = listSS.tellg();
                int length = 0;
                for(;;) {
                    int curr = listSS.get();
                    if(curr == EOF) {
                        break;
                    }
                    if(curr == '\\') {
                        ++length;
                        listSS.get();
                    } else if(curr == '\'') {
                        break;
                    }
                    ++length;
                }
                listSS.seekg(begin);
                string taker(length, ' ');
                listSS.read(&taker[0], length);
                listSS.ignore();
                ans.emplace_back(taker);
            } else {
                listSS.ignore();
                listSS.ignore();
                streampos begin = listSS.tellg();
                int length = 0;
                for(;;) {
                    int curr = listSS.get();
                    if(curr == EOF) {
                        break;
                    } else if(curr == '"' && listSS.peek() == '"') {
                        length++;
                        break;
                    }
                    ++length;
                }
                listSS.seekg(begin);
                string taker(length, ' ');
                listSS.read(&taker[0], length);
                listSS.ignore();
                ans.emplace_back(taker);

            }
            listSS.ignore();
            listSS.ignore();
        }
        return ans;
    }
}
void loadSongs(unordered_map<string, Song>& songMap, unordered_map<string, vector<Artist>>& artists) {
  ifstream file("songs.csv");
  string line;
  getline(file, line);  // get rid of the first line
  while (getline(file, line)) {
    istringstream stream(line);
    string field;
    Song song{};

    // TODO: for artists and artist_ids, need to check for the brackets and
    // multiple artists? (commas and apostrophes
    //  separate different artists; check row 245 for example)

    // For id, name, album, album_id, artists, artist_ids
    
    song.id = getField(stream);
    song.name = getField(stream);
    song.album = getField(stream);
    song.album_id = getField(stream);
    song.artists = splitInternalLists(stream);
    song.artist_ids = splitInternalLists(stream);
    for(int i = 0; i < song.artists.size(); i++) {
        if(artists.find(song.artists[i]) != artists.end()) {
            bool sentinel = false;
            for(auto& artist : artists[song.artists[i]]) {
                if(artist.id == song.artist_ids[i]) {
                    artist.songs.emplace(song.name, song.id);
                    sentinel = true;
                    break;
                }
            }
            if(sentinel) continue;
        }
        artists[song.artists[i]].emplace_back(song.artist_ids[i], song.name, song.id);
    }
    // Discard track number and disc number
    for (int i = 0; i < 2; i++) {
      getField(stream);
    }

    // Store whether song is explicit
    getline(stream, field, ',');
    song.explicit_song = field == "True";

    // For danceability, energy, key, loudness, mode, speechiness, acousticness,
    // instrumentalness, liveliness, valence, tempo

    song.attributes.danceability = stod(getField(stream));
    song.attributes.energy = stod(getField(stream));
    song.attributes.key = stod(getField(stream));
    song.attributes.loudness = stod(getField(stream));
    song.attributes.mode = stod(getField(stream));
    song.attributes.speechiness = stod(getField(stream));
    song.attributes.acousticness = stod(getField(stream));
    song.attributes.instrumentalness = stod(getField(stream));
    song.attributes.liveliness = stod(getField(stream));
    song.attributes.valence = stod(getField(stream));
    song.attributes.tempo = stod(getField(stream));

    // Discard duration
    getField(stream);

    // Initialize time_signature, year
    song.time_signature = stoi(getField(stream));
    song.year = stoi(getField(stream));

    // Don't need to store release date (already store year)

    // Add song to hash map
    songMap[song.id] = song;
  }
}

double calcSimilarity(const songAttributes& s1, const songAttributes& s2) {//https://en.wikipedia.org/wiki/Cosine_similarity
  double product = 0.0;
  double magnitudeS1 = 0.0;
  double magnitudeS2 = 0.0;
  // perform cosine similarity algorithm (measures the cosine of the angle
  // between the two vectors)
  product += s1.danceability * s2.danceability;
  magnitudeS1 += s1.danceability * s1.danceability;
  magnitudeS2 += s2.danceability * s2.danceability;
  product += s1.energy * s2.energy;
  magnitudeS1 += s1.energy * s1.energy;
  magnitudeS2 += s2.energy * s2.energy;
  product += s1.key * s2.key;
  magnitudeS1 += s1.key * s1.key;
  magnitudeS2 += s2.key * s2.key;
  product += s1.loudness * s2.loudness;
  magnitudeS1 += s1.loudness * s1.loudness;
  magnitudeS2 += s2.loudness * s2.loudness;
  product += s1.mode * s2.mode;
  magnitudeS1 += s1.mode * s1.mode;
  magnitudeS2 += s2.mode * s2.mode;
  product += s1.speechiness * s2.speechiness;
  magnitudeS1 += s1.speechiness * s1.speechiness;
  magnitudeS2 += s2.speechiness * s2.speechiness;
  product += s1.acousticness * s2.acousticness;
  magnitudeS1 += s1.acousticness * s1.acousticness;
  magnitudeS2 += s2.acousticness * s2.acousticness;
  product += s1.instrumentalness * s2.instrumentalness;
  magnitudeS1 += s1.instrumentalness * s1.instrumentalness;
  magnitudeS2 += s2.instrumentalness * s2.instrumentalness;
  product += s1.liveliness * s2.liveliness;
  magnitudeS1 += s1.liveliness * s1.liveliness;
  magnitudeS2 += s2.liveliness * s2.liveliness;
  product += s1.valence * s2.valence;
  magnitudeS1 += s1.valence * s1.valence;
  magnitudeS2 += s2.valence * s2.valence;
  product += s1.tempo * s2.tempo;
  magnitudeS1 += s1.tempo * s1.tempo;
  magnitudeS2 += s2.tempo * s2.tempo;

  return product / (sqrt(magnitudeS1) * sqrt(magnitudeS2));
}

string findSongId(const string& name, const string& artist,
                  const unordered_map<string, vector<Artist>>& artistNames) {
  if(artistNames.find(artist) != artistNames.end()) {
    for(const auto& artistToSong : artistNames.at(artist)) {
        if(artistToSong.songs.find(name) != artistToSong.songs.end()) {
            return artistToSong.songs.at(name);
        }
    }
  }
  throw runtime_error("Song not found");
}
unordered_map<string, double> findSimilarSongsDFS(
    const string& songId, const unordered_map<string, Song>& songMap, int trials, int n){
  unordered_set<string> alFound;
  alFound.emplace(songId);//for DFS we want to go down different paths
  Song songSearched = songMap.at(songId);
  string maxSong;
  double maxSim = 0;
  unordered_map<string, double> ansReturn;//return ID and similarity
  for(int i = 0;i < n;i++){//find n songs
    for (int j = 0; j<trials;j++){//go down trials times

    for (const auto& songPair : songMap) {
      // the song that will be compared with the searched song
      const Song &song = songPair.second;
      if (alFound.find(songPair.first) == alFound.end()) {//dont go to a place we've already visited
        // calculate similarity
        double similarity =
            calcSimilarity(songSearched.attributes, song.attributes);
        if (maxSong.empty() || maxSim < similarity) {//if this is the most similar song
          maxSong = songPair.first;
          maxSim = similarity;
        }
      }
    }
    alFound.emplace(maxSong);//we've already been here
    songSearched = songMap.at(maxSong);//DFS start at the song we found
    maxSong = "";
    }
    ansReturn.emplace(songSearched.id, calcSimilarity(songSearched.attributes, songMap.at(songId).attributes));//calculate similarity between found song and original song
    songSearched = songMap.at(songId);
    maxSong = "";
  }
  return ansReturn;
}
unordered_map<string, double> findSimilarSongsBFS(
    const string& songId, const unordered_map<string, Song>& songMap, int N) {
  priority_queue<pair<string, double>, vector<pair<string, double>>,
                 compareSongPairsGreater> //Convert to min-heap, so that pop() removes lowest
      songPQ;
  const Song& songSearched = songMap.at(songId);
  // number of similar songs to keep

  // iterate through each comparison in the songMap
  for (const auto& songPair : songMap) {
    // the song that will be compared with the searched song
    const Song& song = songPair.second;
    // check that the songs have the explicit rating and that they aren't the
    // same song
    if (songId != song.id &&
        songSearched.explicit_song == song.explicit_song) {
      // calculate similarity
      double similarity =
          calcSimilarity(songSearched.attributes, song.attributes);
      // push song into song priority queue
      songPQ.emplace(song.id, similarity);
      // if priority queue is greater than 15, remove the song with the lowest
      // similarity score
      if (songPQ.size() > N) {
        songPQ.pop();
      }
    }
  }
  unordered_map<string, double> similarMap;
  // add the songs into the similarSongs map
  while (!songPQ.empty()) {
    similarMap[songPQ.top().first] = songPQ.top().second;
    songPQ.pop();
  }
  return similarMap;
}
unordered_map<string, double> metaFindSimilarSongs(const string& songId, const unordered_map<string, Song>& songMap, int trials, int n) {
    unordered_map<string, double> ans = findSimilarSongsBFS(songId, songMap, n);
    for (int i = 0; i < trials - 1; ++i) {//n trials
        for(auto& pair : ans) {//BFS for all found songs
            unordered_map<string, double> bfs = findSimilarSongsBFS(pair.first, songMap, n);
            for(const auto& [id, val] : bfs) {
                ans[id] += pow(val, (1.0f / (double)(i+1)));//treat other finds less seriously
            }
        }
        if(ans.find(songId) != ans.end()) ans.erase(songId);//remove curr song
        vector<pair<string, double>> ansVec(ans.begin(), ans.end());
        std::sort(ansVec.begin(), ansVec.end(), compareSongPairsGreater());//sort
        for(auto iter = ansVec.begin() + (n-1); iter != ansVec.end(); ++iter) {
            ans.erase(iter->first);//remove all but n
        }
        
        
    }
    return ans;
}
int main() {
  unordered_map<string, Song> songMap;
  unordered_map<string, vector<Artist>> artistNames;
  loadSongs(songMap, artistNames);

  // read in song input
  string repeat = "yes";
  cout <<"Welcome to the Song Recommendorinator!!!"<< endl;
  while(repeat == "yes") {
      string songName;
      string songArtist;
      cout << "Enter Song Title: ";
      getline(cin, songName);
      cout << "Enter Artist: ";
      getline(cin, songArtist);
      string songId;

      try {
          // search for song in songMap
          songId = findSongId(songName, songArtist, artistNames);
      }
      catch (const exception& error) {
          // if song doesn't exist return error
          cout << error.what() << endl;
          return 1;
      }
  string trial;
  cout << "Enter number of trials: ";
    getline(cin, trial);
  string n;
  string alg;
  cout << "Enter number of similar songs to find: ";
    getline(cin, n);
    int check = stoi(trial);
    int gop = stoi(n);
  cout << "Enter prefered algorithm, BFS(1) or DFS(2): ";
  getline(cin, alg);
      // calculate similar songs
      unordered_map<string, double> similarSongs = alg == "1" ?
          metaFindSimilarSongs(songId, songMap, check, gop): findSimilarSongsDFS(songId, songMap, check, gop);
      for (const auto& [songId, similarity] : similarSongs) {
          const Song& song = songMap.at(songId);
          cout << "Title: " << song.name << endl;
          cout << "Artist: " << song.artists[0] << endl;
          cout << "Similarity: " << similarity << endl;
      }
      cout << "Would you like to search another song? Enter 'yes' or 'no': ";
      getline(cin, repeat);
  }

  return 0;
}
