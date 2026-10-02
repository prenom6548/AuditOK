"""Tests for skill semantic search via embeddings."""

import json
import tempfile
from pathlib import Path
from unittest.mock import MagicMock, patch

import pytest


@pytest.fixture
def mock_config_enabled():
    """Mock config with embedding search enabled."""
    return {
        "embedding_search": {
            "enabled": True,
            "model_path": "/fake/model/path",
            "top_k": 3,
            "min_score": 0.3,
        }
    }


@pytest.fixture
def mock_config_disabled():
    """Mock config with embedding search disabled."""
    return {
        "embedding_search": {
            "enabled": False,
            "model_path": "/fake/model/path",
        }
    }


@pytest.fixture
def mock_skills_data():
    """Mock skill entries for testing."""
    return [
        {
            "name": "postgres-setup",
            "description": "PostgreSQL database setup and configuration",
            "category": "database",
        },
        {
            "name": "python-testing",
            "description": "Python unit testing with pytest",
            "category": "testing",
        },
        {
            "name": "docker-compose",
            "description": "Docker Compose orchestration guide",
            "category": "devops",
        },
    ]


@pytest.fixture
def mock_embeddings_index(mock_skills_data):
    """Mock embeddings index with vectors."""
    return {
        "version": 1,
        "model_path": "/fake/model/path",
        "dim": 4,
        "manifest": {"skill1/SKILL.md": [123456789, 100]},
        "skills": [
            {**skill, "vector": [0.1 * i, 0.2 * i, 0.3 * i, 0.4 * i]}
            for i, skill in enumerate(mock_skills_data, 1)
        ],
    }


class TestConfigLoading:
    """Test configuration loading."""

    @patch("agent.skill_preprocessing.load_skills_config")
    def test_load_config_enabled(self, mock_load, mock_config_enabled):
        """Config enabled returns search config."""
        mock_load.return_value = mock_config_enabled
        from agent.skill_embeddings import _load_embedding_search_config

        cfg = _load_embedding_search_config()
        assert cfg["enabled"] is True
        assert cfg["model_path"] == "/fake/model/path"

    @patch("agent.skill_preprocessing.load_skills_config")
    def test_load_config_disabled(self, mock_load, mock_config_disabled):
        """Config disabled returns empty dict."""
        mock_load.return_value = mock_config_disabled
        from agent.skill_embeddings import _load_embedding_search_config

        cfg = _load_embedding_search_config()
        assert cfg == {}

    @patch("agent.skill_preprocessing.load_skills_config")
    def test_load_config_missing(self, mock_load):
        """Missing config returns empty dict."""
        mock_load.side_effect = Exception("Config error")
        from agent.skill_embeddings import _load_embedding_search_config

        cfg = _load_embedding_search_config()
        assert cfg == {}


class TestModelLoading:
    """Test model loading with caching."""

    @patch("agent.skill_embeddings._MODEL_CACHE", {})
    @patch("agent.skill_embeddings.Path")
    def test_model_path_not_exists(self, mock_path_cls):
        """Non-existent model path returns None."""
        mock_path = MagicMock()
        mock_path.expanduser().resolve().exists.return_value = False
        mock_path_cls.return_value = mock_path

        from agent.skill_embeddings import _get_model

        model = _get_model("/nonexistent/path")
        assert model is None

    @patch("agent.skill_embeddings._MODEL_CACHE", {})
    def test_model_missing_sentence_transformers(self):
        """Missing sentence-transformers returns None."""
        with patch.dict("sys.modules", {"sentence_transformers": None}):
            from agent.skill_embeddings import _get_model

            model = _get_model("/fake/path")
            assert model is None

    @patch("agent.skill_embeddings._MODEL_CACHE", {})
    @patch("agent.skill_embeddings.Path")
    def test_model_load_success_and_cache(self, mock_path_cls):
        """Successful load caches model."""
        mock_path = MagicMock()
        mock_path.expanduser().resolve().exists.return_value = True
        mock_path.expanduser().resolve().__str__.return_value = "/fake/model"
        mock_path_cls.return_value = mock_path

        mock_model = MagicMock()
        mock_model.get_sentence_embedding_dimension.return_value = 768

        from agent.skill_embeddings import _get_model, _MODEL_CACHE

        with patch("sentence_transformers.SentenceTransformer", return_value=mock_model) as mock_st_cls:
            model1 = _get_model("/fake/model")
            assert model1 is mock_model
            assert "/fake/model" in _MODEL_CACHE

            # Second call should use cache (no new SentenceTransformer call)
            mock_st_cls.reset_mock()
            model2 = _get_model("/fake/model")
            assert model2 is mock_model
            mock_st_cls.assert_not_called()


class TestEmbedTexts:
    """Test text embedding."""

    @patch("agent.skill_embeddings._get_model")
    def test_embed_texts_model_unavailable(self, mock_get_model):
        """Model unavailable returns None."""
        mock_get_model.return_value = None
        from agent.skill_embeddings import embed_texts

        result = embed_texts(["test"], "/fake/path")
        assert result is None

    @patch("agent.skill_embeddings._get_model")
    def test_embed_texts_success(self, mock_get_model):
        """Successful encoding returns list of vectors."""
        import numpy as np

        mock_model = MagicMock()
        mock_model.encode.return_value = np.array([[0.1, 0.2], [0.3, 0.4]])
        mock_get_model.return_value = mock_model

        from agent.skill_embeddings import embed_texts

        result = embed_texts(["text1", "text2"], "/fake/path")
        assert result == [[0.1, 0.2], [0.3, 0.4]]

    @patch("agent.skill_embeddings._get_model")
    def test_embed_texts_encoding_fails(self, mock_get_model):
        """Encoding failure returns None."""
        mock_model = MagicMock()
        mock_model.encode.side_effect = RuntimeError("Encoding error")
        mock_get_model.return_value = mock_model

        from agent.skill_embeddings import embed_texts

        result = embed_texts(["text"], "/fake/path")
        assert result is None


class TestLoadSkillEmbeddings:
    """Test loading embeddings index from disk."""

    def test_load_missing_file(self, tmp_path, monkeypatch):
        """Missing index file returns None."""
        monkeypatch.setattr(
            "agent.skill_embeddings.get_hermes_home", lambda: tmp_path
        )
        from agent.skill_embeddings import load_skill_embeddings

        result = load_skill_embeddings()
        assert result is None

    def test_load_version_mismatch(self, tmp_path, monkeypatch, mock_embeddings_index):
        """Version mismatch returns None."""
        index_path = tmp_path / ".skill_embeddings.json"
        bad_index = {**mock_embeddings_index, "version": 999}
        index_path.write_text(json.dumps(bad_index))

        monkeypatch.setattr(
            "agent.skill_embeddings.get_hermes_home", lambda: tmp_path
        )
        from agent.skill_embeddings import load_skill_embeddings

        result = load_skill_embeddings()
        assert result is None

    @patch("agent.skill_embeddings._load_embedding_search_config")
    @patch("agent.skill_embeddings._build_skills_manifest")
    def test_load_model_path_mismatch(
        self, mock_manifest, mock_load_config, tmp_path, monkeypatch, mock_embeddings_index
    ):
        """Model path mismatch returns None."""
        index_path = tmp_path / ".skill_embeddings.json"
        index_path.write_text(json.dumps(mock_embeddings_index))

        monkeypatch.setattr(
            "agent.skill_embeddings.get_hermes_home", lambda: tmp_path
        )
        mock_load_config.return_value = {"model_path": "/different/path"}

        from agent.skill_embeddings import load_skill_embeddings

        result = load_skill_embeddings()
        assert result is None

    @patch("agent.skill_embeddings._load_embedding_search_config")
    @patch("agent.skill_embeddings._build_skills_manifest")
    def test_load_manifest_stale(
        self, mock_manifest, mock_load_config, tmp_path, monkeypatch, mock_embeddings_index
    ):
        """Stale manifest returns None."""
        index_path = tmp_path / ".skill_embeddings.json"
        index_path.write_text(json.dumps(mock_embeddings_index))

        monkeypatch.setattr(
            "agent.skill_embeddings.get_hermes_home", lambda: tmp_path
        )
        mock_load_config.return_value = {"model_path": "/fake/model/path"}
        mock_manifest.return_value = {"different/SKILL.md": [999, 50]}

        from agent.skill_embeddings import load_skill_embeddings

        result = load_skill_embeddings()
        assert result is None

    @patch("agent.skill_embeddings._load_embedding_search_config")
    @patch("agent.skill_embeddings._build_skills_manifest")
    def test_load_success(
        self, mock_manifest, mock_load_config, tmp_path, monkeypatch, mock_embeddings_index
    ):
        """Valid index loads successfully."""
        index_path = tmp_path / ".skill_embeddings.json"
        index_path.write_text(json.dumps(mock_embeddings_index))

        monkeypatch.setattr(
            "agent.skill_embeddings.get_hermes_home", lambda: tmp_path
        )
        mock_load_config.return_value = {"model_path": "/fake/model/path"}
        mock_manifest.return_value = mock_embeddings_index["manifest"]

        from agent.skill_embeddings import load_skill_embeddings

        result = load_skill_embeddings()
        assert result is not None
        assert result["version"] == 1
        assert len(result["skills"]) == 3


class TestCosineSimilarity:
    """Test cosine similarity computation."""

    def test_cosine_identical_vectors(self):
        """Identical vectors have similarity 1.0."""
        from agent.skill_embeddings import _cosine_similarity

        a = [1.0, 2.0, 3.0]
        sim = _cosine_similarity(a, a)
        assert abs(sim - 1.0) < 1e-6

    def test_cosine_orthogonal_vectors(self):
        """Orthogonal vectors have similarity 0.0."""
        from agent.skill_embeddings import _cosine_similarity

        a = [1.0, 0.0]
        b = [0.0, 1.0]
        sim = _cosine_similarity(a, b)
        assert abs(sim) < 1e-6

    def test_cosine_zero_vector(self):
        """Zero vector returns 0.0."""
        from agent.skill_embeddings import _cosine_similarity

        a = [0.0, 0.0]
        b = [1.0, 2.0]
        sim = _cosine_similarity(a, b)
        assert sim == 0.0


class TestSearchSkills:
    """Test skill search with fallback logic."""

    @patch("agent.skill_embeddings._load_embedding_search_config")
    def test_search_config_disabled(self, mock_load_config):
        """Config disabled returns None (fallback)."""
        mock_load_config.return_value = {}
        from agent.skill_embeddings import search_skills

        result = search_skills("postgres setup")
        assert result is None

    @patch("agent.skill_embeddings._load_embedding_search_config")
    @patch("agent.skill_embeddings.load_skill_embeddings")
    def test_search_no_index(self, mock_load_index, mock_load_config):
        """Missing index returns None (fallback)."""
        mock_load_config.return_value = {"model_path": "/fake/path", "top_k": 3, "min_score": 0.3}
        mock_load_index.return_value = None
        from agent.skill_embeddings import search_skills

        result = search_skills("postgres")
        assert result is None

    @patch("agent.skill_embeddings._load_embedding_search_config")
    @patch("agent.skill_embeddings.load_skill_embeddings")
    def test_search_embed_query_fails(self, mock_load_index, mock_load_config, mock_embeddings_index):
        """Query embedding failure returns None (fallback)."""
        mock_load_config.return_value = {"model_path": "/fake/path", "top_k": 3, "min_score": 0.3}
        mock_load_index.return_value = mock_embeddings_index

        from agent.skill_embeddings import search_skills

        with patch("agent.skill_embeddings.embed_texts", return_value=None):
            result = search_skills("postgres")
            assert result is None

    @patch("agent.skill_embeddings._load_embedding_search_config")
    @patch("agent.skill_embeddings.load_skill_embeddings")
    def test_search_below_threshold(self, mock_load_index, mock_load_config, mock_embeddings_index):
        """Top score below threshold returns None (fallback)."""
        mock_load_config.return_value = {"model_path": "/fake/path", "top_k": 3, "min_score": 0.99}
        mock_load_index.return_value = mock_embeddings_index

        from agent.skill_embeddings import search_skills

        # Use a very different vector to get low similarity
        with patch("agent.skill_embeddings.embed_texts", return_value=[[-10.0, -10.0, -10.0, -10.0]]):
            result = search_skills("irrelevant query")
            assert result is None

    @patch("agent.skill_embeddings._load_embedding_search_config")
    @patch("agent.skill_embeddings.load_skill_embeddings")
    def test_search_success(self, mock_load_index, mock_load_config):
        """Successful search returns top-k skills."""
        mock_load_config.return_value = {"model_path": "/fake/path", "top_k": 2, "min_score": 0.3}

        # Create index with distinct vectors for clear ranking
        mock_index = {
            "version": 1,
            "model_path": "/fake/model/path",
            "dim": 4,
            "manifest": {"skill1/SKILL.md": [123456789, 100]},
            "skills": [
                {
                    "name": "postgres-setup",
                    "description": "PostgreSQL database setup",
                    "category": "database",
                    "vector": [1.0, 0.0, 0.0, 0.0],  # Orthogonal to query
                },
                {
                    "name": "python-testing",
                    "description": "Python unit testing",
                    "category": "testing",
                    "vector": [0.0, 1.0, 0.0, 0.0],  # Orthogonal to query
                },
                {
                    "name": "docker-compose",
                    "description": "Docker Compose orchestration",
                    "category": "devops",
                    "vector": [0.0, 0.0, 1.0, 0.0],  # Will match query perfectly
                },
            ],
        }
        mock_load_index.return_value = mock_index

        from agent.skill_embeddings import search_skills

        # Query vector identical to docker-compose
        with patch("agent.skill_embeddings.embed_texts", return_value=[[0.0, 0.0, 1.0, 0.0]]):
            result = search_skills("docker orchestration")
            assert result is not None
            assert len(result) == 2
            assert all("score" in r for r in result)
            # docker-compose should rank highest (score=1.0)
            assert result[0]["name"] == "docker-compose"
            assert result[0]["score"] == 1.0
