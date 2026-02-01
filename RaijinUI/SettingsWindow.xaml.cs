using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

// Raijin ECGI - Settings Configuration Window
// Comprehensive system configuration interface

namespace RaijinUI
{
    public partial class SettingsWindow : Window, INotifyPropertyChanged
    {
        private SettingsData _settingsData;
        private bool _hasChanges;

        public SettingsWindow()
        {
            InitializeComponent();
            LoadSettings();
            DataContext = this;
        }

        #region View Models

        public GeneralSettingsViewModel GeneralSettings => _settingsData.GeneralSettings;
        public EthicsSettingsViewModel EthicsSettings => _settingsData.EthicsSettings;
        public NeuralSettingsViewModel NeuralSettings => _settingsData.NeuralSettings;
        public HardwareSettingsViewModel HardwareSettings => _settingsData.HardwareSettings;
        public ScreenSettingsViewModel ScreenSettings => _settingsData.ScreenSettings;
        public InternetSettingsViewModel InternetSettings => _settingsData.InternetSettings;
        public ProgrammingSettingsViewModel ProgrammingSettings => _settingsData.ProgrammingSettings;
        public SafetySettingsViewModel SafetySettings => _settingsData.SafetySettings;

        #endregion

        #region Settings Management

        private void LoadSettings()
        {
            try
            {
                string settingsPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Config", "settings.json");
                if (File.Exists(settingsPath))
                {
                    string json = File.ReadAllText(settingsPath);
                    _settingsData = System.Text.Json.JsonSerializer.Deserialize<SettingsData>(json);
                }
                else
                {
                    _settingsData = new SettingsData();
                    // Load defaults
                    LoadDefaultSettings();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to load settings: {ex.Message}\nLoading defaults.", "Settings Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                _settingsData = new SettingsData();
                LoadDefaultSettings();
            }

            _hasChanges = false;
        }

        private void LoadDefaultSettings()
        {
            // General Settings Defaults
            _settingsData.GeneralSettings.AutoStart = false;
            _settingsData.GeneralSettings.MinimizeToTray = true;
            _settingsData.GeneralSettings.ShowNotifications = true;
            _settingsData.GeneralSettings.EnableSounds = false;
            _settingsData.GeneralSettings.HardwareAcceleration = true;

            // Ethics Settings Defaults
            _settingsData.EthicsSettings.LearningRate = 1.0;
            _settingsData.EthicsSettings.ValueRetentionDays = 90;
            _settingsData.EthicsSettings.AllowPersonalityEvolution = true;
            _settingsData.EthicsSettings.RequireEthicalConfirmation = false;
            _settingsData.EthicsSettings.LearnFromCorrections = true;

            // Neural Settings Defaults
            _settingsData.NeuralSettings.PopulationSize = 100;
            _settingsData.NeuralSettings.EvolutionGenerations = 10;
            _settingsData.NeuralSettings.EnableGpuAcceleration = true;
            _settingsData.NeuralSettings.UseHyperDimensionalEmbeddings = true;
            _settingsData.NeuralSettings.EnableEntropyComputation = true;

            // Hardware Settings Defaults
            _settingsData.HardwareSettings.CpuUsageLimit = 80;
            _settingsData.HardwareSettings.GpuUsageLimit = 90;
            _settingsData.HardwareSettings.MemoryUsageLimit = 85;
            _settingsData.HardwareSettings.AllowDynamicFrequencyScaling = true;
            _settingsData.HardwareSettings.EnableThermalManagement = true;
            _settingsData.HardwareSettings.AllowFirmwareModifications = false;

            // Screen Settings Defaults
            _settingsData.ScreenSettings.CaptureIntervalMs = 100;
            _settingsData.ScreenSettings.EnableRealTimeAnalysis = true;
            _settingsData.ScreenSettings.AllowAutomatedControl = true;
            _settingsData.ScreenSettings.UseHumanLikePatterns = true;
            _settingsData.ScreenSettings.EnableOcr = true;

            // Internet Settings Defaults
            _settingsData.InternetSettings.DailyKnowledgeLimitMb = 1000;
            _settingsData.InternetSettings.EnableAutonomousBrowsing = true;
            _settingsData.InternetSettings.RespectRobotsTxt = true;
            _settingsData.InternetSettings.UseAnonymization = true;
            _settingsData.InternetSettings.EnableMultiModalProcessing = true;

            // Programming Settings Defaults
            _settingsData.ProgrammingSettings.EnableCodeAnalysis = true;
            _settingsData.ProgrammingSettings.AllowAutonomousCodeGeneration = true;
            _settingsData.ProgrammingSettings.EnableCompilerOptimization = true;
            _settingsData.ProgrammingSettings.AllowFirmwareModification = false;
            _settingsData.ProgrammingSettings.EnableSelfModification = false;

            // Safety Settings Defaults
            _settingsData.SafetySettings.EnableEmergencyStop = true;
            _settingsData.SafetySettings.RequireConfirmationForDangerousOps = true;
            _settingsData.SafetySettings.EnableAutomaticBackups = true;
            _settingsData.SafetySettings.LogAllActivities = true;
            _settingsData.SafetySettings.EnableHardwareWatchdog = true;
        }

        private void SaveSettings()
        {
            try
            {
                string configDir = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Config");
                Directory.CreateDirectory(configDir);

                string settingsPath = Path.Combine(configDir, "settings.json");
                var options = new System.Text.Json.JsonSerializerOptions { WriteIndented = true };
                string json = System.Text.Json.JsonSerializer.Serialize(_settingsData, options);
                File.WriteAllText(settingsPath, json);

                _hasChanges = false;
                MessageBox.Show("Settings saved successfully!", "Settings Saved", MessageBoxButton.OK, MessageBoxImage.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to save settings: {ex.Message}", "Save Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        #endregion

        #region Event Handlers

        private void SaveSettingsButton_Click(object sender, RoutedEventArgs e)
        {
            SaveSettings();
            DialogResult = true;
            Close();
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            if (_hasChanges)
            {
                var result = MessageBox.Show("You have unsaved changes. Discard them?", "Unsaved Changes",
                                           MessageBoxButton.YesNo, MessageBoxImage.Question);
                if (result != MessageBoxResult.Yes) return;
            }

            DialogResult = false;
            Close();
        }

        private void ResetToDefaultsButton_Click(object sender, RoutedEventArgs e)
        {
            var result = MessageBox.Show("Reset all settings to defaults? This cannot be undone.", "Reset to Defaults",
                                       MessageBoxButton.YesNo, MessageBoxImage.Warning);
            if (result == MessageBoxResult.Yes)
            {
                LoadDefaultSettings();
                _hasChanges = true;
                OnPropertyChanged(nameof(GeneralSettings));
                OnPropertyChanged(nameof(EthicsSettings));
                OnPropertyChanged(nameof(NeuralSettings));
                OnPropertyChanged(nameof(HardwareSettings));
                OnPropertyChanged(nameof(ScreenSettings));
                OnPropertyChanged(nameof(InternetSettings));
                OnPropertyChanged(nameof(ProgrammingSettings));
                OnPropertyChanged(nameof(SafetySettings));
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

    #region Settings Data Models

    public class SettingsData
    {
        public GeneralSettingsViewModel GeneralSettings { get; set; } = new GeneralSettingsViewModel();
        public EthicsSettingsViewModel EthicsSettings { get; set; } = new EthicsSettingsViewModel();
        public NeuralSettingsViewModel NeuralSettings { get; set; } = new NeuralSettingsViewModel();
        public HardwareSettingsViewModel HardwareSettings { get; set; } = new HardwareSettingsViewModel();
        public ScreenSettingsViewModel ScreenSettings { get; set; } = new ScreenSettingsViewModel();
        public InternetSettingsViewModel InternetSettings { get; set; } = new InternetSettingsViewModel();
        public ProgrammingSettingsViewModel ProgrammingSettings { get; set; } = new ProgrammingSettingsViewModel();
        public SafetySettingsViewModel SafetySettings { get; set; } = new SafetySettingsViewModel();
    }

    public class GeneralSettingsViewModel : INotifyPropertyChanged
    {
        private bool _autoStart;
        private bool _minimizeToTray;
        private bool _showNotifications;
        private bool _enableSounds;
        private bool _hardwareAcceleration;

        public bool AutoStart { get => _autoStart; set { _autoStart = value; OnPropertyChanged(); } }
        public bool MinimizeToTray { get => _minimizeToTray; set { _minimizeToTray = value; OnPropertyChanged(); } }
        public bool ShowNotifications { get => _showNotifications; set { _showNotifications = value; OnPropertyChanged(); } }
        public bool EnableSounds { get => _enableSounds; set { _enableSounds = value; OnPropertyChanged(); } }
        public bool HardwareAcceleration { get => _hardwareAcceleration; set { _hardwareAcceleration = value; OnPropertyChanged(); } }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class EthicsSettingsViewModel : INotifyPropertyChanged
    {
        private double _learningRate;
        private int _valueRetentionDays;
        private bool _allowPersonalityEvolution;
        private bool _requireEthicalConfirmation;
        private bool _learnFromCorrections;

        public double LearningRate { get => _learningRate; set { _learningRate = value; OnPropertyChanged(); } }
        public int ValueRetentionDays { get => _valueRetentionDays; set { _valueRetentionDays = value; OnPropertyChanged(); } }
        public bool AllowPersonalityEvolution { get => _allowPersonalityEvolution; set { _allowPersonalityEvolution = value; OnPropertyChanged(); } }
        public bool RequireEthicalConfirmation { get => _requireEthicalConfirmation; set { _requireEthicalConfirmation = value; OnPropertyChanged(); } }
        public bool LearnFromCorrections { get => _learnFromCorrections; set { _learnFromCorrections = value; OnPropertyChanged(); } }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class NeuralSettingsViewModel : INotifyPropertyChanged
    {
        private int _populationSize;
        private int _evolutionGenerations;
        private bool _enableGpuAcceleration;
        private bool _useHyperDimensionalEmbeddings;
        private bool _enableEntropyComputation;

        public int PopulationSize { get => _populationSize; set { _populationSize = value; OnPropertyChanged(); } }
        public int EvolutionGenerations { get => _evolutionGenerations; set { _evolutionGenerations = value; OnPropertyChanged(); } }
        public bool EnableGpuAcceleration { get => _enableGpuAcceleration; set { _enableGpuAcceleration = value; OnPropertyChanged(); } }
        public bool UseHyperDimensionalEmbeddings { get => _useHyperDimensionalEmbeddings; set { _useHyperDimensionalEmbeddings = value; OnPropertyChanged(); } }
        public bool EnableEntropyComputation { get => _enableEntropyComputation; set { _enableEntropyComputation = value; OnPropertyChanged(); } }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class HardwareSettingsViewModel : INotifyPropertyChanged
    {
        private int _cpuUsageLimit;
        private int _gpuUsageLimit;
        private int _memoryUsageLimit;
        private bool _allowDynamicFrequencyScaling;
        private bool _enableThermalManagement;
        private bool _allowFirmwareModifications;

        public int CpuUsageLimit { get => _cpuUsageLimit; set { _cpuUsageLimit = value; OnPropertyChanged(); } }
        public int GpuUsageLimit { get => _gpuUsageLimit; set { _gpuUsageLimit = value; OnPropertyChanged(); } }
        public int MemoryUsageLimit { get => _memoryUsageLimit; set { _memoryUsageLimit = value; OnPropertyChanged(); } }
        public bool AllowDynamicFrequencyScaling { get => _allowDynamicFrequencyScaling; set { _allowDynamicFrequencyScaling = value; OnPropertyChanged(); } }
        public bool EnableThermalManagement { get => _enableThermalManagement; set { _enableThermalManagement = value; OnPropertyChanged(); } }
        public bool AllowFirmwareModifications { get => _allowFirmwareModifications; set { _allowFirmwareModifications = value; OnPropertyChanged(); } }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ScreenSettingsViewModel : INotifyPropertyChanged
    {
        private int _captureIntervalMs;
        private bool _enableRealTimeAnalysis;
        private bool _allowAutomatedControl;
        private bool _useHumanLikePatterns;
        private bool _enableOcr;

        public int CaptureIntervalMs { get => _captureIntervalMs; set { _captureIntervalMs = value; OnPropertyChanged(); } }
        public bool EnableRealTimeAnalysis { get => _enableRealTimeAnalysis; set { _enableRealTimeAnalysis = value; OnPropertyChanged(); } }
        public bool AllowAutomatedControl { get => _allowAutomatedControl; set { _allowAutomatedControl = value; OnPropertyChanged(); } }
        public bool UseHumanLikePatterns { get => _useHumanLikePatterns; set { _useHumanLikePatterns = value; OnPropertyChanged(); } }
        public bool EnableOcr { get => _enableOcr; set { _enableOcr = value; OnPropertyChanged(); } }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class InternetSettingsViewModel : INotifyPropertyChanged
    {
        private int _dailyKnowledgeLimitMb;
        private bool _enableAutonomousBrowsing;
        private bool _respectRobotsTxt;
        private bool _useAnonymization;
        private bool _enableMultiModalProcessing;

        public int DailyKnowledgeLimitMb { get => _dailyKnowledgeLimitMb; set { _dailyKnowledgeLimitMb = value; OnPropertyChanged(); } }
        public bool EnableAutonomousBrowsing { get => _enableAutonomousBrowsing; set { _enableAutonomousBrowsing = value; OnPropertyChanged(); } }
        public bool RespectRobotsTxt { get => _respectRobotsTxt; set { _respectRobotsTxt = value; OnPropertyChanged(); } }
        public bool UseAnonymization { get => _useAnonymization; set { _useAnonymization = value; OnPropertyChanged(); } }
        public bool EnableMultiModalProcessing { get => _enableMultiModalProcessing; set { _enableMultiModalProcessing = value; OnPropertyChanged(); } }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class ProgrammingSettingsViewModel : INotifyPropertyChanged
    {
        private bool _enableCodeAnalysis;
        private bool _allowAutonomousCodeGeneration;
        private bool _enableCompilerOptimization;
        private bool _allowFirmwareModification;
        private bool _enableSelfModification;

        public bool EnableCodeAnalysis { get => _enableCodeAnalysis; set { _enableCodeAnalysis = value; OnPropertyChanged(); } }
        public bool AllowAutonomousCodeGeneration { get => _allowAutonomousCodeGeneration; set { _allowAutonomousCodeGeneration = value; OnPropertyChanged(); } }
        public bool EnableCompilerOptimization { get => _enableCompilerOptimization; set { _enableCompilerOptimization = value; OnPropertyChanged(); } }
        public bool AllowFirmwareModification { get => _allowFirmwareModification; set { _allowFirmwareModification = value; OnPropertyChanged(); } }
        public bool EnableSelfModification { get => _enableSelfModification; set { _enableSelfModification = value; OnPropertyChanged(); } }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class SafetySettingsViewModel : INotifyPropertyChanged
    {
        private bool _enableEmergencyStop;
        private bool _requireConfirmationForDangerousOps;
        private bool _enableAutomaticBackups;
        private bool _logAllActivities;
        private bool _enableHardwareWatchdog;

        public bool EnableEmergencyStop { get => _enableEmergencyStop; set { _enableEmergencyStop = value; OnPropertyChanged(); } }
        public bool RequireConfirmationForDangerousOps { get => _requireConfirmationForDangerousOps; set { _requireConfirmationForDangerousOps = value; OnPropertyChanged(); } }
        public bool EnableAutomaticBackups { get => _enableAutomaticBackups; set { _enableAutomaticBackups = value; OnPropertyChanged(); } }
        public bool LogAllActivities { get => _logAllActivities; set { _logAllActivities = value; OnPropertyChanged(); } }
        public bool EnableHardwareWatchdog { get => _enableHardwareWatchdog; set { _enableHardwareWatchdog = value; OnPropertyChanged(); } }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    #endregion
}