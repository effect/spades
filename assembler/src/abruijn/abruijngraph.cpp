#include <iostream>
#include <fstream>
#include <assert.h>
#include "logging.hpp"
#include "abruijngraph.hpp"
#include "graphVisualizer.hpp"
#include <ext/hash_map>

LOGGER("a.graph");

using namespace abruijn;

namespace abruijn {

ostream& operator<< (ostream& os, const Profile& p) {
	return p.output(os);
}

ostream& operator<< (ostream& os, const Edge& e) {
	return e.output(os);
}

ostream& operator<< (ostream& os, const Vertex& v) {
	return v.output(os);
//	#ifdef OUTPUT_PAIRED
//		os << v.data_.Subseq(0, LABEL).str() << "_" << v.size();
//	#endif
//	#ifndef OUTPUT_PAIRED
//		os << v.data_.Subseq(0, LABEL).str() << "_" << v.size() << "_" << v.data_.Subseq(data_.size() - LABEL).str();
//	#endif
}

Vertex* Graph::createVertex(const Sequence& kmer) {
	LOG_ASSERT(!hasVertex(kmer), "already contains " << kmer.str());
	Vertex* v = new Vertex(kmer, true);
	seqVertice[kmer] = v;
	seqVertice[!kmer] = v->complement();
	vertices.insert(v);
	vertices.insert(v->complement());
	return v;
}

//Vertex* Graph::createVertex(const Sequence& kmer, size_t size) {
//	Vertex* v = createVertex(kmer);
//	v->setSize(size);
//	v->complement()->setSize(size);
//	return v;
//}


void Graph::addEdge(Vertex* from, Vertex* to, const Sequence& seq) {
	from->addEdge(to, seq);
	to->complement()->addEdge(from->complement(), !seq);
}

void Graph::removeVertex(Vertex* v) {
	removeVertex_single(v);
	removeVertex_single(v->complement());
}
void Graph::removeVertex_single(Vertex* v) {
	for (Edges::iterator it = v->edges().begin(); it != v->edges().end(); ++it) {
		it->first->complement()->edges().erase(v->complement());
	}
	removedVertices.insert(v);
}

bool Graph::hasVertex(const Sequence& kmer) {
	return seqVertice.count(kmer);
}

Vertex* Graph::getVertex(const Sequence& kmer) {
	SeqVertice::iterator v = seqVertice.find(kmer);
	if (v == seqVertice.end()) {
		return createVertex(kmer);
	}
	if (v->second->data() == kmer) {
		return v->second;
	}
	assert(v->second->complement()->data() == kmer);
	return v->second->complement();
}

bool Graph::tryCondenseA(Vertex* v) {
	if (v->degree() != 1 || v->complement()->degree() != 1) {
		return false;
	}
//	Vertex* w = v->complement()->edges_.begin()->first->complement();
//	Vertex* u = v->edges_.begin()->first;
//	for (;;) {
//
//	}
	return true;


//	Edge e = v->edges_.begin()->second;
//	assert(u->complement()->edges_.begin()->first == v->complement());
//	Edge f = u->complement()->edges_.begin()->second;
//	TRACE(e << " " << f);
//	if ((e.lengths_.size() != 1) || (e.lengths_.size() != 1)) {
//		ERROR("Unsure what to do");
//		return NULL;
//	}
//	size_t len = e.lengths_.begin()->first;
//	assert(len == f.lengths_.begin()->first);
//	Vertex* vu = createVertex(v->concat(u), len);
//	for (Vertex::Edges::iterator it = u->edges_.begin(); it != u->edges_.end(); ++it) {
//		Vertex* w = it->first;
//		for (map<size_t, int>::iterator it2 = it->second.lengths_.begin(); it2 != it->second.lengths_.end(); ++it2) {
//			size_t l = len + it2->first - u->size();
//			vu->edges_[w].lengths_[l] += it2->second;
//			w->complement()->edges_[vu->complement()].lengths_[l] += it2->second;
//		}
//	}
//	for (Vertex::Edges::iterator it = v->complement()->edges_.begin(); it != v->complement()->edges_.end(); ++it) {
//		Vertex* w = it->first;
//		for (map<size_t, int>::iterator it2 = it->second.lengths_.begin(); it2 != it->second.lengths_.end(); ++it2) {
//			size_t l = len + it2->first - v->size();
//			vu->complement()->edges_[w].lengths_[l] += it2->second;
//			w->complement()->edges_[vu].lengths_[l] += it2->second;
//		}
//	}
//	k++;
//	removeVertex(v);
//	removeVertex(u);
//	return vu;
}

void Graph::cleanup() {
	for (Vertices::iterator v = removedVertices.begin(); v != removedVertices.end(); ++v) {
		vertices.erase(*v);
		delete *v;
	}
	removedVertices.clear();
}

void Graph::output(std::ofstream &out) {
	std::string name = "A_Bruijn_Graph";
//	std::string name = OUTPUT_FILE;
#ifdef OUTPUT_PAIRED
	gvis::PairedGraphPrinter<Vertex*> printer(name, out);
#endif
#ifndef OUTPUT_PAIRED
	gvis::GraphPrinter<Vertex*> printer(name, out);
#endif
	for (Vertices::iterator v = vertices.begin(); v != vertices.end(); ++v) {
		#ifdef OUTPUT_PAIRED
			printer.addVertex(*v, toString(**v), (*v)->complement(), toString(*((*v)->complement())));
		#endif
		#ifndef OUTPUT_PAIRED
			printer.addVertex(*v, toString(**v));
		#endif
		for (Edges::iterator it = (*v)->edges().begin(); it != (*v)->edges().end(); ++it) {
			#ifdef OUTPUT_PAIRED
				printer.addEdge(make_pair(*v, (*v)->complement()), make_pair(it->first, it->first->complement()), toString(it->second));
			#endif
			#ifndef OUTPUT_PAIRED
				printer.addEdge(*v, it->first, it->second);
			#endif
		}
	}
	printer.output();
	out.close();
}

void Graph::output(string filename) {
	ofstream out(filename.c_str(), ios::out);
	output(out);
}

}
