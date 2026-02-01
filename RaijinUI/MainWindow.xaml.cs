using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using Microsoft.WindowsAPICodePack.Shell;
using Microsoft.WindowsAPICodePack.Shell.PropertySystem;

// Raijin ECGI - Main UI Window
// Provides real-time consciousness monitoring and system control

namespace RaijinUI
{
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        // Core system interfaces
        private RaijinCore _raijinCore;
        private ScreenCaptureManager _screenCapture;
        private SystemMonitor _systemMonitor;
        private DispatcherTimer _updateTimer;

        // View Models
        private SystemStatusViewModel _systemStatus;
        private HardwareStatusViewModel _hardwareStatus;
        private PerformanceMetricsViewModel _performanceMetrics;
        private NeuralStatusViewModel _neuralStatus;
        private EthicsStatusViewModel _ethicsStatus;
        private ThoughtStreamViewModel _thoughtStream;
        private ChatInterfaceViewModel _chatInterface;
        private TaskManagerViewModel _taskManager;
        private WorkspaceViewModel _workspace;
        private ScreenControlViewModel _screenControl;
        private SystemGraphsViewModel _systemGraphs;

        public MainWindow()
        {
            InitializeComponent();
            InitializeViewModels();
            InitializeCoreSystems();
            InitializeTimers();
            DataContext = this;
        }

        #region View Models

        public SystemStatusViewModel SystemStatus => _systemStatus;
        public HardwareStatusViewModel HardwareStatus => _hardwareStatus;
        public PerformanceMetricsViewModel PerformanceMetrics => _performanceMetrics;
        public NeuralStatusViewModel NeuralStatus => _neuralStatus;
        public EthicsStatusViewModel EthicsStatus => _ethicsStatus;
        public ThoughtStreamViewModel ThoughtStream => _thoughtStream;
        public ChatInterfaceViewModel ChatInterface => _chatInterface;
        public TaskManagerViewModel TaskManager => _taskManager;
        public WorkspaceViewModel Workspace => _workspace;
        public ScreenControlViewModel ScreenControl => _screenControl;
        public SystemGraphsViewModel SystemGraphs => _systemGraphs;

        #endregion

        #region Initialization

        private void InitializeViewModels()
        {
            _systemStatus = new SystemStatusViewModel();
            _hardwareStatus = new HardwareStatusViewModel();
            _performanceMetrics = new PerformanceMetricsViewModel();
            _neuralStatus = new NeuralStatusViewModel();
            _ethicsStatus = new EthicsStatusViewModel();
            _thoughtStream = new ThoughtStreamViewModel();
            _chatInterface = new ChatInterfaceViewModel();
            _taskManager = new TaskManagerViewModel();
            _workspace = new WorkspaceViewModel();
            _screenControl = new ScreenControlViewModel();
            _systemGraphs = new SystemGraphsViewModel();
        }

        private void InitializeCoreSystems()
        {
            try
            {
                // Initialize Raijin core system
                _raijinCore = new RaijinCore();
                _raijinCore.ThoughtGenerated += OnThoughtGenerated;
                _raijinCore.TaskUpdated += OnTaskUpdated;
                _raijinCore.SystemStatusChanged += OnSystemStatusChanged;

                // Initialize screen capture
                _screenCapture = new ScreenCaptureManager();
                _screenCapture.ScreenCaptured += OnScreenCaptured;

                // Initialize system monitor
                _systemMonitor = new SystemMonitor();
                _systemMonitor.HardwareStatusUpdated += OnHardwareStatusUpdated;

                // Start core system
                _raijinCore.Start();

                AddThought("System initialized successfully", "SYSTEM");
                AddChatMessage("Raijin", "Consciousness online. Ready for interaction.");
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to initialize Raijin core: {ex.Message}",
                              "Initialization Error", MessageBoxButton.OK, MessageBoxImage.Error);
                Application.Current.Shutdown();
            }
        }

        private void InitializeTimers()
        {
            _updateTimer = new DispatcherTimer();
            _updateTimer.Interval = TimeSpan.FromMilliseconds(100); // 10 FPS updates
            _updateTimer.Tick += UpdateTimer_Tick;
            _updateTimer.Start();
        }

        #endregion

        #region Event Handlers

        private void UpdateTimer_Tick(object sender, EventArgs e)
        {
            // Update all view models with latest data
            UpdateSystemStatus();
            UpdateHardwareStatus();
            UpdatePerformanceMetrics();
            UpdateNeuralStatus();
            UpdateEthicsStatus();
            UpdateSystemGraphs();

            // Auto-scroll thought stream
            if (_thoughtStream.Thoughts.Count > 0)
            {
                ThoughtStreamListBox.ScrollIntoView(_thoughtStream.Thoughts[_thoughtStream.Thoughts.Count - 1]);
            }

            // Auto-scroll chat
            if (_chatInterface.Messages.Count > 0)
            {
                ChatHistoryListBox.ScrollIntoView(_chatInterface.Messages[_chatInterface.Messages.Count - 1]);
            }
        }

        private void OnThoughtGenerated(object sender, ThoughtEventArgs e)
        {
            Dispatcher.Invoke(() => AddThought(e.Content, e.Type));
        }

        private void OnTaskUpdated(object sender, TaskEventArgs e)
        {
            Dispatcher.Invoke(() => UpdateTask(e.Task));
        }

        private void OnSystemStatusChanged(object sender, SystemStatusEventArgs e)
        {
            Dispatcher.Invoke(() => _systemStatus.UpdateFromCore(e.Status));
        }

        private void OnScreenCaptured(object sender, ScreenCaptureEventArgs e)
        {
            Dispatcher.Invoke(() => UpdateScreenImage(e.Image));
        }

        private void OnHardwareStatusUpdated(object sender, HardwareStatusEventArgs e)
        {
            Dispatcher.Invoke(() => _hardwareStatus.UpdateFromMonitor(e.Status));
        }

        #endregion

        #region UI Event Handlers

        private void SettingsButton_Click(object sender, RoutedEventArgs e)
        {
            var settingsWindow = new SettingsWindow();
            settingsWindow.Owner = this;
            settingsWindow.ShowDialog();
        }

        private void StartAutonomousButton_Click(object sender, RoutedEventArgs e)
        {
            _raijinCore.StartAutonomousOperation();
            AddThought("Autonomous operation started", "CONTROL");
        }

        private void StopAutonomousButton_Click(object sender, RoutedEventArgs e)
        {
            _raijinCore.StopAutonomousOperation();
            AddThought("Autonomous operation stopped", "CONTROL");
        }

        private void PauseResumeButton_Click(object sender, RoutedEventArgs e)
        {
            _raijinCore.TogglePauseResume();
            AddThought("System pause/resume toggled", "CONTROL");
        }

        private void EmergencyStopButton_Click(object sender, RoutedEventArgs e)
        {
            var result = MessageBox.Show("Emergency stop will halt all operations. Continue?",
                                       "Emergency Stop", MessageBoxButton.YesNo, MessageBoxImage.Warning);
            if (result == MessageBoxResult.Yes)
            {
                _raijinCore.EmergencyStop();
                AddThought("EMERGENCY STOP ACTIVATED", "CRITICAL");
            }
        }

        private void ApplyAutonomyButton_Click(object sender, RoutedEventArgs e)
        {
            double autonomyLevel = AutonomySlider.Value;
            _raijinCore.SetAutonomyLevel(autonomyLevel);
            AddThought($"Autonomy level set to {autonomyLevel:P0}", "CONTROL");
        }

        private void CaptureScreenButton_Click(object sender, RoutedEventArgs e)
        {
            _screenCapture.CaptureScreen();
            AddThought("Screen capture initiated", "SCREEN");
        }

        private void StartControlButton_Click(object sender, RoutedEventArgs e)
        {
            _screenCapture.StartControl();
            _screenControl.StatusText = "Control Active";
            AddThought("Screen control started", "SCREEN");
        }

        private void StopControlButton_Click(object sender, RoutedEventArgs e)
        {
            _screenCapture.StopControl();
            _screenControl.StatusText = "Control Inactive";
            AddThought("Screen control stopped", "SCREEN");
        }

        private void NewProjectButton_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new NewProjectDialog();
            if (dialog.ShowDialog() == true)
            {
                _raijinCore.CreateNewProject(dialog.ProjectName, dialog.ProjectType);
                AddThought($"New project created: {dialog.ProjectName}", "PROJECT");
            }
        }

        private void AnalyzeCodeButton_Click(object sender, RoutedEventArgs e)
        {
            _raijinCore.AnalyzeCode();
            AddThought("Code analysis initiated", "ANALYSIS");
        }

        private void GenerateReportButton_Click(object sender, RoutedEventArgs e)
        {
            _raijinCore.GenerateReport();
            AddThought("Report generation started", "REPORT");
        }

        private void OptimizeSystemButton_Click(object sender, RoutedEventArgs e)
        {
            _raijinCore.OptimizeSystem();
            AddThought("System optimization initiated", "OPTIMIZATION");
        }

        private void BackupDataButton_Click(object sender, RoutedEventArgs e)
        {
            _raijinCore.BackupData();
            AddThought("Data backup started", "BACKUP");
        }

        private void SendButton_Click(object sender, RoutedEventArgs e)
        {
            SendMessage(interrupt: true);
        }

        private void SendNoInterruptButton_Click(object sender, RoutedEventArgs e)
        {
            SendMessage(interrupt: false);
        }

        private void ChatInputTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter && (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
            {
                SendMessage(interrupt: false);
                e.Handled = true;
            }
        }

        private void Window_Closing(object sender, CancelEventArgs e)
        {
            var result = MessageBox.Show("Closing Raijin will stop all autonomous operations. Continue?",
                                       "Confirm Exit", MessageBoxButton.YesNo, MessageBoxImage.Question);

            if (result != MessageBoxResult.Yes)
            {
                e.Cancel = true;
                return;
            }

            // Graceful shutdown
            _updateTimer.Stop();
            _raijinCore?.Shutdown();
            _screenCapture?.Dispose();
            _systemMonitor?.Dispose();
        }

        #endregion

        #region Helper Methods

        private void AddThought(string content, string type)
        {
            var thought = new Thought
            {
                Timestamp = DateTime.Now.ToString("HH:mm:ss"),
                Content = content,
                Type = type
            };
            _thoughtStream.Thoughts.Add(thought);
        }

        private void AddChatMessage(string sender, string content)
        {
            var message = new ChatMessage
            {
                Timestamp = DateTime.Now,
                Sender = sender,
                Content = content
            };
            _chatInterface.Messages.Add(message);
        }

        private void SendMessage(bool interrupt)
        {
            string message = _chatInterface.CurrentMessage?.Trim();
            if (string.IsNullOrEmpty(message)) return;

            AddChatMessage("User", message);

            if (interrupt)
            {
                _raijinCore.ProcessMessageInterrupt(message);
            }
            else
            {
                _raijinCore.ProcessMessageQueued(message);
            }

            _chatInterface.CurrentMessage = string.Empty;
        }

        private void UpdateTask(TaskInfo task)
        {
            var existingTask = _taskManager.ActiveTasks.FirstOrDefault(t => t.Id == task.Id);
            if (existingTask != null)
            {
                existingTask.Progress = task.Progress;
                existingTask.Status = task.Status;
            }
            else
            {
                _taskManager.ActiveTasks.Add(task);
            }
        }

        private void UpdateScreenImage(Bitmap bitmap)
        {
            var bitmapImage = ConvertBitmapToBitmapImage(bitmap);
            ScreenImage.Source = bitmapImage;
        }

        private void UpdateSystemStatus()
        {
            if (_raijinCore != null)
            {
                var status = _raijinCore.GetSystemStatus();
                _systemStatus.UpdateFromCore(status);
            }
        }

        private void UpdateHardwareStatus()
        {
            if (_systemMonitor != null)
            {
                var status = _systemMonitor.GetHardwareStatus();
                _hardwareStatus.UpdateFromMonitor(status);
            }
        }

        private void UpdatePerformanceMetrics()
        {
            if (_raijinCore != null)
            {
                var metrics = _raijinCore.GetPerformanceMetrics();
                _performanceMetrics.UpdateFromCore(metrics);
            }
        }

        private void UpdateNeuralStatus()
        {
            if (_raijinCore != null)
            {
                var status = _raijinCore.GetNeuralStatus();
                _neuralStatus.UpdateFromCore(status);
            }
        }

        private void UpdateEthicsStatus()
        {
            if (_raijinCore != null)
            {
                var status = _raijinCore.GetEthicsStatus();
                _ethicsStatus.UpdateFromCore(status);
            }
        }

        private void UpdateSystemGraphs()
        {
            if (_raijinCore != null)
            {
                var graphs = _raijinCore.GetSystemGraphs();
                _systemGraphs.UpdateFromCore(graphs);
            }
        }

        private BitmapImage ConvertBitmapToBitmapImage(Bitmap bitmap)
        {
            using (var memory = new MemoryStream())
            {
                bitmap.Save(memory, ImageFormat.Png);
                memory.Position = 0;
                var bitmapImage = new BitmapImage();
                bitmapImage.BeginInit();
                bitmapImage.StreamSource = memory;
                bitmapImage.CacheOption = BitmapCacheOption.OnLoad;
                bitmapImage.EndInit();
                return bitmapImage;
            }
        }

        #endregion

        #region INotifyPropertyChanged

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion
    }

    #region View Models

    public class SystemStatusViewModel : INotifyPropertyChanged
    {
        private string _stateText = "Initializing...";
        private string _autonomyText = "Level: 70%";
        private string _userPresenceText = "User: Present";
        private string _tasksQueuedText = "Tasks: 0";
        private string _uptimeText = "Uptime: 00:00:00";

        public string StateText { get => _stateText; set { _stateText = value; OnPropertyChanged(); } }
        public string AutonomyText { get => _autonomyText; set { _autonomyText = value; OnPropertyChanged(); } }
        public string UserPresenceText { get => _userPresenceText; set { _userPresenceText = value; OnPropertyChanged(); } }
        public string TasksQueuedText { get => _tasksQueuedText; set { _tasksQueuedText = value; OnPropertyChanged(); } }
        public string UptimeText { get => _uptimeText; set { _uptimeText = value; OnPropertyChanged(); } }

        public void UpdateFromCore(SystemStatus status)
        {
            StateText = status.State;
            AutonomyText = $"Level: {status.AutonomyLevel:P0}";
            UserPresenceText = $"User: {(status.UserPresent ? "Present" : "Away")}";
            TasksQueuedText = $"Tasks: {status.TasksQueued}";
            UptimeText = $"Uptime: {status.Uptime:hh\\:mm\\:ss}";
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class HardwareStatusViewModel : INotifyPropertyChanged
    {
        private string _cpuText = "CPU: Analyzing...";
        private string _gpuText = "GPU: Analyzing...";
        private string _memoryText = "RAM: Analyzing...";
        private string _storageText = "SSD: Analyzing...";
        private string _networkText = "NET: Analyzing...";

        public string CpuText { get => _cpuText; set { _cpuText = value; OnPropertyChanged(); } }
        public string GpuText { get => _gpuText; set { _gpuText = value; OnPropertyChanged(); } }
        public string MemoryText { get => _memoryText; set { _memoryText = value; OnPropertyChanged(); } }
        public string StorageText { get => _storageText; set { _storageText = value; OnPropertyChanged(); } }
        public string NetworkText { get => _networkText; set { _networkText = value; OnPropertyChanged(); } }

        public void UpdateFromMonitor(HardwareStatus status)
        {
            CpuText = $"CPU: {status.CpuUsage:F1}% ({status.CpuTemp:F0}°C)";
            GpuText = $"GPU: {status.GpuUsage:F1}% ({status.GpuTemp:F0}°C)";
            MemoryText = $"RAM: {status.MemoryUsed:F1}GB / {status.MemoryTotal:F1}GB";
            StorageText = $"SSD: {status.StorageUsed:F1}GB / {status.StorageTotal:F1}GB";
            NetworkText = $"NET: {status.NetworkUp:F1}↑ {status.NetworkDown:F1}↓ Mbps";
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class PerformanceMetricsViewModel : INotifyPropertyChanged
    {
        private string _tasksCompletedText = "Completed: 0";
        private string _successRateText = "Success: 0%";
        private string _averageResponseTimeText = "Response: 0ms";
        private string _systemEfficiencyText = "Efficiency: 0%";

        public string TasksCompletedText { get => _tasksCompletedText; set { _tasksCompletedText = value; OnPropertyChanged(); } }
        public string SuccessRateText { get => _successRateText; set { _successRateText = value; OnPropertyChanged(); } }
        public string AverageResponseTimeText { get => _averageResponseTimeText; set { _averageResponseTimeText = value; OnPropertyChanged(); } }
        public string SystemEfficiencyText { get => _systemEfficiencyText; set { _systemEfficiencyText = value; OnPropertyChanged(); } }

        public void UpdateFromCore(PerformanceMetrics metrics)
        {
            TasksCompletedText = $"Completed: {metrics.TasksCompleted}";
            SuccessRateText = $"Success: {metrics.SuccessRate:P0}";
            AverageResponseTimeText = $"Response: {metrics.AverageResponseTime:F0}ms";
            SystemEfficiencyText = $"Efficiency: {metrics.SystemEfficiency:P0}";
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class NeuralStatusViewModel : INotifyPropertyChanged
    {
        private string _entropyText = "Entropy: 0.000";
        private string _connectionsText = "Connections: 0";
        private string _evolutionText = "Evolution: 0";
        private string _confidenceText = "Confidence: 0%";

        public string EntropyText { get => _entropyText; set { _entropyText = value; OnPropertyChanged(); } }
        public string ConnectionsText { get => _connectionsText; set { _connectionsText = value; OnPropertyChanged(); } }
        public string EvolutionText { get => _evolutionText; set { _evolutionText = value; OnPropertyChanged(); } }
        public string ConfidenceText { get => _confidenceText; set { _confidenceText = value; OnPropertyChanged(); } }

        public void UpdateFromCore(NeuralStatus status)
        {
            EntropyText = $"Entropy: {status.Entropy:F3}";
            ConnectionsText = $"Connections: {status.Connections:N0}";
            EvolutionText = $"Evolution: {status.EvolutionGeneration}";
            ConfidenceText = $"Confidence: {status.Confidence:P0}";
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class EthicsStatusViewModel : INotifyPropertyChanged
    {
        private string _alignmentText = "Alignment: 0%";
        private string _valuesLearnedText = "Values: 0";
        private string _personalityText = "Personality: Unknown";
        private double _alignmentScore;

        public string AlignmentText { get => _alignmentText; set { _alignmentText = value; OnPropertyChanged(); } }
        public string ValuesLearnedText { get => _valuesLearnedText; set { _valuesLearnedText = value; OnPropertyChanged(); } }
        public string PersonalityText { get => _personalityText; set { _personalityText = value; OnPropertyChanged(); } }
        public double AlignmentScore { get => _alignmentScore; set { _alignmentScore = value; OnPropertyChanged(); } }

        public void UpdateFromCore(EthicsStatus status)
        {
            AlignmentText = $"Alignment: {status.Alignment:P0}";
            ValuesLearnedText = $"Values: {status.ValuesLearned}";
            PersonalityText = $"Personality: {status.PersonalityType}";
            AlignmentScore = status.Alignment;
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ThoughtStreamViewModel : INotifyPropertyChanged
    {
        public ObservableCollection<Thought> Thoughts { get; } = new ObservableCollection<Thought>();

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ChatInterfaceViewModel : INotifyPropertyChanged
    {
        private string _currentMessage = "";

        public ObservableCollection<ChatMessage> Messages { get; } = new ObservableCollection<ChatMessage>();
        public string CurrentMessage { get => _currentMessage; set { _currentMessage = value; OnPropertyChanged(); } }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class TaskManagerViewModel : INotifyPropertyChanged
    {
        public ObservableCollection<TaskInfo> ActiveTasks { get; } = new ObservableCollection<TaskInfo>();

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class WorkspaceViewModel : INotifyPropertyChanged
    {
        public ObservableCollection<FileItem> Files { get; } = new ObservableCollection<FileItem>();

        public WorkspaceViewModel()
        {
            LoadWorkspace();
        }

        private void LoadWorkspace()
        {
            // Load files from Raijin workspace directory
            var workspacePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Workspace");
            if (Directory.Exists(workspacePath))
            {
                LoadDirectory(workspacePath, Files);
            }
        }

        private void LoadDirectory(string path, ObservableCollection<FileItem> parent)
        {
            try
            {
                foreach (var dir in Directory.GetDirectories(path))
                {
                    var dirItem = new FileItem
                    {
                        Name = Path.GetFileName(dir),
                        Path = dir,
                        IsDirectory = true,
                        Icon = "/Resources/folder.ico"
                    };
                    parent.Add(dirItem);
                    LoadDirectory(dir, dirItem.Children);
                }

                foreach (var file in Directory.GetFiles(path))
                {
                    var fileItem = new FileItem
                    {
                        Name = Path.GetFileName(file),
                        Path = file,
                        IsDirectory = false,
                        Icon = GetFileIcon(file)
                    };
                    parent.Add(fileItem);
                }
            }
            catch (Exception ex)
            {
                // Handle access errors gracefully
                Debug.WriteLine($"Error loading directory {path}: {ex.Message}");
            }
        }

        private string GetFileIcon(string filePath)
        {
            var ext = Path.GetExtension(filePath).ToLower();
            return ext switch
            {
                ".cs" => "/Resources/cs.ico",
                ".cpp" => "/Resources/cpp.ico",
                ".h" => "/Resources/h.ico",
                ".xaml" => "/Resources/xaml.ico",
                ".md" => "/Resources/md.ico",
                ".txt" => "/Resources/txt.ico",
                _ => "/Resources/file.ico"
            };
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ScreenControlViewModel : INotifyPropertyChanged
    {
        private string _statusText = "Inactive";

        public string StatusText { get => _statusText; set { _statusText = value; OnPropertyChanged(); } }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class SystemGraphsViewModel : INotifyPropertyChanged
    {
        private double _learningProgress;
        private double _neuralActivity;
        private double _systemEfficiency;
        private double _evolutionRate;

        public double LearningProgress { get => _learningProgress; set { _learningProgress = value; OnPropertyChanged(); } }
        public double NeuralActivity { get => _neuralActivity; set { _neuralActivity = value; OnPropertyChanged(); } }
        public double SystemEfficiency { get => _systemEfficiency; set { _systemEfficiency = value; OnPropertyChanged(); } }
        public double EvolutionRate { get => _evolutionRate; set { _evolutionRate = value; OnPropertyChanged(); } }

        public void UpdateFromCore(SystemGraphs graphs)
        {
            LearningProgress = graphs.LearningProgress;
            NeuralActivity = graphs.NeuralActivity;
            SystemEfficiency = graphs.SystemEfficiency;
            EvolutionRate = graphs.EvolutionRate;
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    #endregion

    #region Data Models

    public class Thought
    {
        public string Timestamp { get; set; }
        public string Content { get; set; }
        public string Type { get; set; }
    }

    public class ChatMessage
    {
        public DateTime Timestamp { get; set; }
        public string Sender { get; set; }
        public string Content { get; set; }
    }

    public class TaskInfo
    {
        public int Id { get; set; }
        public string Description { get; set; }
        public double Progress { get; set; }
        public string Status { get; set; }
    }

    public class FileItem
    {
        public string Name { get; set; }
        public string Path { get; set; }
        public bool IsDirectory { get; set; }
        public string Icon { get; set; }
        public ObservableCollection<FileItem> Children { get; } = new ObservableCollection<FileItem>();
    }

    #endregion

    #region Core System Interfaces

    // These classes interface with the C/C++ core systems
    // Implementation details would connect to actual core libraries

    public class RaijinCore
    {
        public event EventHandler<ThoughtEventArgs> ThoughtGenerated;
        public event EventHandler<TaskEventArgs> TaskUpdated;
        public event EventHandler<SystemStatusEventArgs> SystemStatusChanged;

        public void Start() { /* Initialize core systems */ }
        public void Shutdown() { /* Shutdown core systems */ }
        public void StartAutonomousOperation() { /* Start autonomous mode */ }
        public void StopAutonomousOperation() { /* Stop autonomous mode */ }
        public void TogglePauseResume() { /* Toggle pause/resume */ }
        public void EmergencyStop() { /* Emergency stop */ }
        public void SetAutonomyLevel(double level) { /* Set autonomy level */ }
        public void ProcessMessageInterrupt(string message) { /* Process message with interrupt */ }
        public void ProcessMessageQueued(string message) { /* Process message queued */ }
        public void CreateNewProject(string name, string type) { /* Create new project */ }
        public void AnalyzeCode() { /* Analyze code */ }
        public void GenerateReport() { /* Generate report */ }
        public void OptimizeSystem() { /* Optimize system */ }
        public void BackupData() { /* Backup data */ }

        public SystemStatus GetSystemStatus() => new SystemStatus();
        public PerformanceMetrics GetPerformanceMetrics() => new PerformanceMetrics();
        public NeuralStatus GetNeuralStatus() => new NeuralStatus();
        public EthicsStatus GetEthicsStatus() => new EthicsStatus();
        public SystemGraphs GetSystemGraphs() => new SystemGraphs();
    }

    public class ScreenCaptureManager : IDisposable
    {
        public event EventHandler<ScreenCaptureEventArgs> ScreenCaptured;

        public void CaptureScreen() { /* Capture screen */ }
        public void StartControl() { /* Start screen control */ }
        public void StopControl() { /* Stop screen control */ }
        public void Dispose() { /* Cleanup resources */ }
    }

    public class SystemMonitor : IDisposable
    {
        public event EventHandler<HardwareStatusEventArgs> HardwareStatusUpdated;

        public HardwareStatus GetHardwareStatus() => new HardwareStatus();
        public void Dispose() { /* Cleanup resources */ }
    }

    #endregion

    #region Event Args

    public class ThoughtEventArgs : EventArgs
    {
        public string Content { get; set; }
        public string Type { get; set; }
    }

    public class TaskEventArgs : EventArgs
    {
        public TaskInfo Task { get; set; }
    }

    public class SystemStatusEventArgs : EventArgs
    {
        public SystemStatus Status { get; set; }
    }

    public class ScreenCaptureEventArgs : EventArgs
    {
        public Bitmap Image { get; set; }
    }

    public class HardwareStatusEventArgs : EventArgs
    {
        public HardwareStatus Status { get; set; }
    }

    #endregion

    #region Data Structures

    public class SystemStatus
    {
        public string State { get; set; } = "Active";
        public double AutonomyLevel { get; set; } = 0.7;
        public bool UserPresent { get; set; } = true;
        public int TasksQueued { get; set; } = 0;
        public TimeSpan Uptime { get; set; } = TimeSpan.Zero;
    }

    public class HardwareStatus
    {
        public double CpuUsage { get; set; } = 0;
        public double CpuTemp { get; set; } = 0;
        public double GpuUsage { get; set; } = 0;
        public double GpuTemp { get; set; } = 0;
        public double MemoryUsed { get; set; } = 0;
        public double MemoryTotal { get; set; } = 32;
        public double StorageUsed { get; set; } = 0;
        public double StorageTotal { get; set; } = 932;
        public double NetworkUp { get; set; } = 0;
        public double NetworkDown { get; set; } = 0;
    }

    public class PerformanceMetrics
    {
        public int TasksCompleted { get; set; } = 0;
        public double SuccessRate { get; set; } = 0;
        public double AverageResponseTime { get; set; } = 0;
        public double SystemEfficiency { get; set; } = 0;
    }

    public class NeuralStatus
    {
        public double Entropy { get; set; } = 0;
        public long Connections { get; set; } = 0;
        public int EvolutionGeneration { get; set; } = 0;
        public double Confidence { get; set; } = 0;
    }

    public class EthicsStatus
    {
        public double Alignment { get; set; } = 0;
        public int ValuesLearned { get; set; } = 0;
        public string PersonalityType { get; set; } = "Unknown";
    }

    public class SystemGraphs
    {
        public double LearningProgress { get; set; } = 0;
        public double NeuralActivity { get; set; } = 0;
        public double SystemEfficiency { get; set; } = 0;
        public double EvolutionRate { get; set; } = 0;
    }

    #endregion
}