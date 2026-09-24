const state = {
  entities: [],
  events: [],
  observations: [],
  relationships: [],
  sources: [],
  graph: { nodes: [], edges: [] },
  timeline: [],
  status: {},
};

function setText(id, value) {
  const el = document.getElementById(id);
  if (el) {
    el.textContent = value;
  }
}

function renderStatus(status) {
  state.status = status;
  setText('engine-status', `ENGINE ${status.engine_status || 'ONLINE'}`);
  setText('world-time', status.world_time || '--');
  setText('entity-count', status.entity_count ?? 0);
  setText('event-count', status.event_count ?? 0);
  setText('observation-count', status.observation_count ?? 0);
  setText('relationship-count', status.relationship_count ?? 0);
  setText('source-count', status.source_count ?? 0);
  setText('metric-entities', status.entity_count ?? 0);
  setText('metric-events', status.event_count ?? 0);
  setText('metric-observations', status.observation_count ?? 0);
  setText('metric-relationships', status.relationship_count ?? 0);
}

function renderList(id, items, formatter, emptyMessage = 'No data') {
  const container = document.getElementById(id);
  if (!container) return;
  container.innerHTML = '';
  if (!items.length) {
    container.innerHTML = `<div class="list-item"><strong>${emptyMessage}</strong></div>`;
    return;
  }
  for (const item of items) {
    container.appendChild(formatter(item));
  }
}

function entityFormatter(entity) {
  const div = document.createElement('div');
  div.className = 'list-item';
  const label = document.createElement('strong');
  label.textContent = entity.canonical_name || entity.id;
  const detail = document.createElement('small');
  detail.textContent = `${entity.entity_type || 'unknown'} • ${entity.description || ''}`;
  div.appendChild(label);
  div.appendChild(detail);
  return div;
}

function eventFormatter(event) {
  const div = document.createElement('div');
  div.className = 'list-item';
  const label = document.createElement('strong');
  label.textContent = event.event_type || 'event';
  const detail = document.createElement('small');
  detail.textContent = `${event.timestamp || 0} • ${event.description || ''}`;
  div.appendChild(label);
  div.appendChild(detail);
  return div;
}

function observationFormatter(observation) {
  const div = document.createElement('div');
  div.className = 'list-item';
  const label = document.createElement('strong');
  label.textContent = observation.data_type || 'observation';
  const detail = document.createElement('small');
  detail.textContent = `${observation.timestamp || 0} • ${observation.content || ''}`;
  div.appendChild(label);
  div.appendChild(detail);
  return div;
}

function renderGraph(graph) {
  const svg = document.getElementById('graph');
  if (!svg) return;
  svg.innerHTML = '';

  const width = 900;
  const height = 320;
  const centerX = width / 2;
  const centerY = height / 2;
  const radius = 26;

  const nodes = graph.nodes || [];
  const edges = graph.edges || [];

  edges.forEach((edge, index) => {
    const source = nodes.find((node) => node.id === edge.source);
    const target = nodes.find((node) => node.id === edge.target);
    if (!source || !target) return;
    const sourceX = centerX + ((index % 3) - 1) * 180 + (source.type === 'company' ? 0 : 80);
    const sourceY = centerY + ((index % 2) * 80) - 40;
    const targetX = centerX + ((index % 3) - 1) * 180 + 120;
    const targetY = centerY + ((index % 2) * 80) - 10;
    const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
    line.setAttribute('x1', sourceX);
    line.setAttribute('y1', sourceY);
    line.setAttribute('x2', targetX);
    line.setAttribute('y2', targetY);
    line.setAttribute('stroke', '#5cc8ff');
    line.setAttribute('stroke-width', '2');
    svg.appendChild(line);
  });

  nodes.forEach((node, index) => {
    const x = centerX + ((index % 3) - 1) * 180;
    const y = centerY + (Math.floor(index / 3) * 80) - 40;
    const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
    circle.setAttribute('cx', x);
    circle.setAttribute('cy', y);
    circle.setAttribute('r', radius);
    circle.setAttribute('fill', node.type === 'company' ? '#5cc8ff' : '#8af2d2');
    circle.setAttribute('stroke', '#dfeaf9');
    svg.appendChild(circle);

    const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    text.setAttribute('x', x);
    text.setAttribute('y', y + 4);
    text.setAttribute('text-anchor', 'middle');
    text.setAttribute('fill', '#0b1020');
    text.setAttribute('font-size', '11');
    text.setAttribute('font-weight', 'bold');
    text.textContent = node.label.slice(0, 10);
    svg.appendChild(text);
  });
}

function renderTimeline(items) {
  const timeline = document.getElementById('timeline');
  if (!timeline) return;
  timeline.innerHTML = '';
  items.forEach((entry) => {
    const row = document.createElement('div');
    row.className = 'timeline-item';
    row.innerHTML = `<span class="timeline-dot"></span><span>${entry.timestamp} • ${entry.label}</span>`;
    timeline.appendChild(row);
  });
}

function renderSearchResults(results) {
  const box = document.getElementById('search-results');
  if (!box) return;
  box.innerHTML = '';
  if (!results.length) {
    box.innerHTML = '<div class="result-item">No results</div>';
    return;
  }
  results.forEach((result) => {
    const item = document.createElement('div');
    item.className = 'result-item';
    item.innerHTML = `<strong>${result.type}</strong><div>${result.label}</div><small>${result.detail}</small>`;
    box.appendChild(item);
  });
}

async function fetchJson(url) {
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(`Request failed: ${url}`);
  }
  return response.json();
}

async function loadDashboard() {
  try {
    const status = await fetchJson('/api/status');
    renderStatus(status);

    const [entities, events, observations, graph, timelineData] = await Promise.all([
      fetchJson('/api/entities'),
      fetchJson('/api/events'),
      fetchJson('/api/observations'),
      fetchJson('/api/graph'),
      fetchJson('/api/timeline'),
    ]);

    state.entities = entities;
    state.events = events;
    state.observations = observations;
    state.graph = graph;
    state.timeline = timelineData;

    renderList('entity-list', entities, entityFormatter);
    renderList('event-list', events, eventFormatter);
    renderList('observation-list', observations, observationFormatter);
    renderGraph(graph);
    renderTimeline(timelineData);
  } catch (error) {
    console.error(error);
  }
}

function bindSearch() {
  const input = document.getElementById('search-input');
  const button = document.getElementById('search-button');

  const runSearch = async () => {
    const q = input.value.trim();
    if (!q) return;
    try {
      const results = await fetchJson(`/api/search?q=${encodeURIComponent(q)}`);
      renderSearchResults(results);
    } catch (error) {
      console.error(error);
    }
  };

  button.addEventListener('click', runSearch);
  input.addEventListener('keydown', (event) => {
    if (event.key === 'Enter') runSearch();
  });
}

document.addEventListener('DOMContentLoaded', () => {
  bindSearch();
  loadDashboard();
  setInterval(loadDashboard, 5000);
});
